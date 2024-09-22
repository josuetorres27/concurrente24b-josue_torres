// Copyright 2024 Josué Torres Sibaja <josue.torressibaja@ucr.ac.cr>

#include "heat_sim.h"  // NOLINT

void configure_simulation(SimulationData* sim_params, const char* job_file,
  const char* dir, uint64_t job_lines, uint64_t num_threads) {
  FILE* file;
  char filepath[MAX_PATH_LENGTH];
  SharedData* shared_data = malloc(sizeof(SharedData));
  if (shared_data == NULL) {
    fprintf(stderr, "Error allocating memory for shared data\n");
    return EXIT_FAILURE;
  }

  // Arreglo para almacenar los estados finales de cada simulación
  uint64_t* sim_states = malloc(job_lines * sizeof(uint64_t));
  if (sim_states == NULL) {
    fprintf(stderr, "Error allocating memory for simulation states\n");
    free(shared_data);  // Libera la memoria de shared_data antes de retornar
    return EXIT_FAILURE;
  }

  for (uint64_t job_index = 0; job_index < job_lines; job_index++) {
    // Construye la ruta completa al archivo binario de entrada
    snprintf(filepath, sizeof(filepath), "%s/%s", dir,
      sim_params[job_index].filename);

    // Abre el archivo binario
    file = fopen(filepath, "rb");
    if (file == NULL) {
      fprintf(stderr, "Error opening binary file %s\n",
        sim_params[job_index].filename);
      free(sim_states);
      free(shared_data);
      return EXIT_FAILURE;
    }

    // Lee el número de filas y columnas de la lámina
    fread(&(shared_data->rows), sizeof(uint64_t), 1, file);
    fread(&(shared_data->cols), sizeof(uint64_t), 1, file);

    // Asigna memoria para la matriz de la lámina
    shared_data->data = malloc(shared_data->rows * sizeof(double*));
    if (shared_data->data == NULL) {
      fprintf(stderr, "Error allocating memory for plate rows\n");
      fclose(file);
      free(sim_states);
      free(shared_data);
      return EXIT_FAILURE;
    }

    // Asigna memoria para cada fila de la matriz
    for (uint64_t row = 0; row < shared_data->rows; row++) {
      shared_data->data[row] = malloc(shared_data->cols * sizeof(double));
      if (shared_data->data[row] == NULL) {
        fprintf(stderr, "Error allocating memory for plate columns\n");
        // Liberación de memoria en caso de error
        for (uint64_t r = 0; r < row; r++) {
          free(shared_data->data[r]);
        }
        free(shared_data->data);
        fclose(file);
        free(sim_states);
        free(shared_data);
        return EXIT_FAILURE;
      }
    }

    // Lee los valores de la lámina del archivo binario
    for (uint64_t row = 0; row < shared_data->rows; row++) {
      if (fread(shared_data->data[row], sizeof(double), shared_data->cols,
        file) != shared_data->cols) {
        fprintf(stderr, "Error reading plate data of file %s\n",
          sim_params[job_index].filename);
        // Liberación de memoria en caso de error
        for (uint64_t r = 0; r < shared_data->rows; r++) {
          free(shared_data->data[r]);
        }
        free(shared_data->data);
        fclose(file);
        free(sim_states);
        free(shared_data);
        return EXIT_FAILURE;
      }
    }

    // Configura los parámetros de simulación en SharedData
    shared_data->delta_t = sim_params[job_index].delta_t;
    shared_data->alpha = sim_params[job_index].alpha;
    shared_data->h = sim_params[job_index].h;
    shared_data->epsilon = sim_params[job_index].epsilon;

    // Llama a la función de simulación
    uint64_t num_states = simulate(shared_data, num_threads);

    // Guarda el número de estados finales de la simulación
    sim_states[job_index] = num_states;

    // Genera el archivo binario con los resultados de la simulación
    write_plate(shared_data->data, shared_data->rows, shared_data->cols, dir,
      sim_params[job_index].filename, num_states);

    // Libera la memoria de la lámina
    for (uint64_t row = 0; row < shared_data->rows; row++) {
      free(shared_data->data[row]);
    }
    free(shared_data->data);
    // Cierra el archivo binario
    fclose(file);
  }

  // Genera el reporte en formato .tsv
  create_report(dir, job_file, sim_params, sim_states, job_lines);

  // Libera memoria
  free(sim_states);
  free(shared_data);
}

uint64_t simulate(SharedData* shared_data, uint64_t num_threads) {
  pthread_t threads[num_threads];
  ThreadData thread_data[num_threads];

  // Inicialización de variables
  uint64_t num_states = 0;  // Número de estados simulados
  bool eq_point = false;  // Punto de equilibrio alcanzado

  // Ciclo principal de simulación hasta alcanzar el equilibrio
  while (!eq_point) {
    num_states++;  // Incrementa el contador de estados
    eq_point = true;  // Asumimos que el equilibrio se alcanza en este paso
    // Crear una copia de la matriz actual antes de actualizar
    double** internal_data = malloc(shared_data->rows * sizeof(double*));
    for (uint64_t i = 0; i < shared_data->rows; i++) {
      internal_data[i] = malloc(shared_data->cols * sizeof(double));
      for (uint64_t j = 0; j < shared_data->cols; j++) {
        internal_data[i][j] = shared_data->data[i][j];
      }
    }

    // Dividir el trabajo entre los hilos
    uint64_t rows_per_thread = (shared_data->rows - 2) / num_threads;
    for (uint64_t i = 0; i < num_threads; i++) {
      thread_data[i].start_row = 1 + i * rows_per_thread;
      thread_data[i].end_row = (i == num_threads - 1) ? shared_data->rows - 1 :
        thread_data[i].start_row + rows_per_thread;
      thread_data[i].shared_data = shared_data;

      // Crear los hilos
      pthread_create(&threads[i], NULL, thread_heat_sim, &thread_data[i]);
    }
    // Esperar a que todos los hilos terminen
    for (uint64_t i = 0; i < num_threads; i++) {
      pthread_join(threads[i], NULL);
    }
    // Verificar si se ha alcanzado el equilibrio
    for (uint64_t i = 1; i < shared_data->rows - 1; i++) {
      for (uint64_t j = 1; j < shared_data->cols - 1; j++) {
        double temp = internal_data[i][j];  // Valor anterior
        double next_temp = shared_data->data[i][j];  // Valor actualizado
        if (fabs(next_temp - temp) >= shared_data->epsilon) {
          eq_point = false;
          break;
        }
      }
      if (!eq_point) break;
    }
    // Liberar la memoria de internal_data
    for (uint64_t i = 0; i < shared_data->rows; i++) {
      free(internal_data[i]);
    }
    free(internal_data);
  }
  // Retornar el número de estados
  return num_states;
}

void* thread_heat_sim(void* data) {
  ThreadData* thread_data = (ThreadData*) data;
  SharedData* shared_data = thread_data->shared_data;

  // Procesar solo la parte de la data asignada a este hilo
  for (uint64_t i = thread_data->start_row; i < thread_data->end_row; i++) {
    for (uint64_t j = 1; j < shared_data->cols - 1; j++) {
      double temp = shared_data->data[i][j];
      double next_temp = temp + ((shared_data->delta_t * shared_data->alpha) /
        (shared_data->h * shared_data->h)) * (shared_data->data[i-1][j] +
          shared_data->data[i+1][j] + shared_data->data[i][j-1] +
            shared_data->data[i][j+1] - 4 * temp);
      shared_data->data[i][j] = next_temp;  // Actualizar la data compartida
    }
  }
  return NULL;  // Fin del thread
}
