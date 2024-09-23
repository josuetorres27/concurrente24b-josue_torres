// Copyright 2024 Josué Torres Sibaja <josue.torressibaja@ucr.ac.cr>

#include "heat_simulation.h"  // NOLINT

/**
 * @brief Configura y ejecuta la simulación de calor para una serie de láminas.
 * 
 * @details Esta función realiza las siguientes operaciones para cada lámina:
 * - Lee los datos de la lámina desde un archivo binario.
 * - Asigna la memoria necesaria para las filas y columnas.
 * - Configura los parámetros de la simulación.
 * - Ejecuta la simulación utilizando múltiples hilos.
 * - Escribe los resultados de la simulación en un archivo binario.
 * - Genera un reporte final en formato .tsv.
 * 
 * @param dir Directorio donde se encuentran los archivos.
 * @param sim_params Arreglo de estructuras que contiene los parámetros de
 * simulación para cada lámina.
 * @param lines Número de láminas térmicas a simular.
 * @param job_name Nombre del archivo de trabajo, utilizado para generar el
 * reporte.
 * @param thread_count Número de hilos a utilizar en la simulación.
 */
void configure_simulation(const char* dir, SimData* sim_params, uint64_t lines,
  const char* job_name, uint64_t thread_count) {
  FILE* file;  /** Puntero para manejar la apertura y lectura de archivos. */
  char adress[512];  /** Buffer para almacenar la ruta del archivo. */
  /** Asignación dinámica para los datos compartidos. */
  SharedData* shared_data = malloc(1 * sizeof(SharedData));
  if (shared_data == NULL) {
    fprintf(stderr, "Error allocating memory for shared data\n");
    return;
  }

  /** Arreglo para almacenar el estado de la simulación para cada lámina. */
  uint64_t* sim_states = malloc(lines * sizeof(uint64_t));

  /** Bucle que recorre todas las laminas de simulación. */
  for (uint64_t i = 0; i < lines; i++) {
    /** Construir la ruta completa del archivo binario. */
    snprintf(adress, sizeof(adress), "%s/%s", dir, sim_params[i].bin_name);
    file = fopen(adress, "rb");
    if (file == NULL) {
      fprintf(stderr, "Error opening binary file: %s\n",
        sim_params[i].bin_name);
      return;
    }
    /** Asignar memoria para las filas y columnas de la lámina. */
    fread(&(shared_data->rows), sizeof(uint64_t), 1, file);
    fread(&(shared_data->cols), sizeof(uint64_t), 1, file);
    shared_data->data = malloc(shared_data->rows * sizeof(double*));
    if (shared_data->data == NULL) {
      fprintf(stderr, "Error allocating memory for plate rows\n");
      fclose(file);  /** Cerrar el archivo para evitar pérdidas de memoria. */
      return;
    }
    for (uint64_t i = 0; i < shared_data->rows; i++) {
      shared_data->data[i] = malloc(shared_data->cols * sizeof(double));
      if (shared_data->data[i] == NULL) {
        fprintf(stderr, "Error allocating memory for plate cols\n");
        /** Liberar memoria si ocurre un error. */
        for (uint64_t j = 0; j < i; j++) {
          free(shared_data->data[j]);
        }
        free(shared_data->data);
        fclose(file);
        return;
      }
    }
    /** Leer los datos de la lámina. */
    for (uint64_t i = 0; i < shared_data->rows; i++) {
      if (fread(shared_data->data[i], sizeof(double), shared_data->cols, file)
        != shared_data->cols) {
        fprintf(stderr, "Error reading plate data: %s\n",
          sim_params[i].bin_name);
        /** Si ocurre un error, libera la memoria y cierra el archivo. */
        for (uint64_t j = 0; j < shared_data->rows; j++) {
          free(shared_data->data[j]);
        }
        free(shared_data->data);
        fclose(file);
        return;
      }
    }
    /** Configurar los parámetros de la simulación. */
    shared_data->delta_t = sim_params[i].delta_t;
    shared_data->alpha = sim_params[i].alpha;
    shared_data->h = sim_params[i].h;
    shared_data->epsilon = sim_params[i].epsilon;

    /**
     * Llamar a la función de simulación que devuelve el número de estados
     * necesarios para estabilizar.
     */
    uint64_t num_states;
    num_states = simulate(shared_data, thread_count);
    sim_states[i] = num_states;

    /** Escribir los resultados de la simulación en un archivo binario. */
    write_plate(shared_data->data, shared_data->rows, shared_data->cols, dir,
      sim_params[i].bin_name, num_states);
    /** Liberar la memoria asignada a los datos de la lámina. */
    for (uint64_t j = 0; j < shared_data->rows; j++) {
      free(shared_data->data[j]);
    }
    free(shared_data->data);
    fclose(file);
  }

  /** Generar el archivo de reporte. */
  create_report(dir, job_name, sim_params, sim_states, lines);

  /** Liberar la memoria. */
  free(sim_states);
  free(shared_data);
}

/**
 * @brief Ejecuta la simulación de calor utilizando múltiples hilos hasta
 * alcanzar el equilibrio térmico.
 * 
 * @details Esta función coordina la simulación del calor en una lámina
 * representada por una matriz de datos. La matriz se divide en secciones que
 * son procesadas en paralelo por varios hilos. La simulación continúa hasta
 * que se alcanza el equilibrio térmico.
 * 
 * @param shared_data Puntero a la estructura que contiene los parámetros y la
 * matriz compartida de la simulación.
 * @param thread_count Número de hilos a utilizar para ejecutar la simulación
 * en paralelo.
 * 
 * @return Número de iteraciones necesarias para alcanzar el equilibrio
 * térmico.
 */
uint64_t simulate(SharedData* shared_data, uint64_t thread_count) {
  /** Se reserva memoria para manejar los hilos y los datos privados. */
  pthread_t threads[thread_count];
  ThreadData thread_data[thread_count];

  uint64_t num_states = 0;
  bool eq_point = false;

  /** Bucle que ejecuta la simulación hasta alcanzar el equilibrio térmico. */
  while (!eq_point) {
    num_states++;  /** Se incrementa el número de iteraciones. */
    eq_point = true;
    /** Crear una copia local de la matriz de datos compartida. */
    double data_local[shared_data->rows][shared_data->cols];
    for (uint64_t i = 0; i < shared_data->rows; i++) {
      for (uint64_t j = 0; j < shared_data->cols; j++) {
        data_local[i][j] = shared_data->data[i][j];
      }
    }

    /** Asignación de filas a cada hilo y creación de los hilos. */
    for (uint64_t i = 0; i < thread_count; i++) {
      /** Calcula el número de filas que manejará cada hilo. */
      uint64_t rows_per_thread = (shared_data->rows - 2) / thread_count;

      /** Asignar la fila de inicio para el hilo 'i'. */
      thread_data[i].start_row = 1 + i * rows_per_thread;

      /** Asignar la fila de finalización. */
      if (i == thread_count - 1) {
        /** El último hilo maneja hasta la última fila. */
        thread_data[i].end_row = shared_data->rows - 1;
      } else {
        thread_data[i].end_row = thread_data[i].start_row + rows_per_thread;
      }
      thread_data[i].shared_data = shared_data;

      /** Crear el hilo, ejecutando la función para actualizar la matriz. */
      pthread_create(&threads[i], NULL, thread_sim, &thread_data[i]);
    }

    /** Esperar a que todos los hilos terminen su ejecución. */
    for (uint64_t i = 0; i < thread_count; i++) {
      pthread_join(threads[i], NULL);
    }

    /** Comprobar si se ha alcanzado el equilibrio térmico. */
    for (uint64_t i = 1; i < shared_data->rows - 1; i++) {
      for (uint64_t j = 1; j < shared_data->cols - 1; j++) {
        double temperature = data_local[i][j];
        double next_temp = shared_data->data[i][j];
        if (fabs(next_temp - temperature) >= shared_data->epsilon) {
          eq_point = false;  /** Se actualiza la bandera de equilibrio. */
          break;
        }
      }
    }
  }
  /** Retornar el número de estados necesarios para alcanzar el equilibrio. */
  return num_states;
}

/**
 * @brief Función que ejecuta la simulación de calor para una sección de la
 * matriz.
 * 
 * @details Esta función es llamada por cada hilo para procesar una parte de la
 * matriz compartida, calculando las nuevas temperaturas basadas en las celdas
 * vecinas. Cada hilo trabaja en un subconjunto de filas de la matriz, asignado
 * a través de la estructura 'ThreadData'.
 * 
 * @param data Puntero a la estructura que contiene la información específica
 * del hilo, como las filas que debe procesar y un puntero a los datos
 * compartidos.
 */
void* thread_sim(void* data) {
  ThreadData* private_data = (ThreadData*) data;
  SharedData* shared_data = private_data->shared_data;

  /** Crear una copia temporal de la matriz para actualizar los valores. */
  double** temp_data = malloc(shared_data->rows * sizeof(double*));
  for (uint64_t i = 0; i < shared_data->rows; i++) {
    temp_data[i] = malloc(shared_data->cols * sizeof(double));
  }
  /**
   * Procesar las filas asignadas a este hilo. Cada hilo maneja una parte de
   * las filas de la matriz.
   */
  for (uint64_t i = private_data->start_row; i < private_data->end_row; i++) {
    for (uint64_t j = 1; j < shared_data->cols - 1; j++) {
      /** Obtener el valor actual de la celda en la posición [i][j]. */
      double temperature = shared_data->data[i][j];

      /** Calcular la nueva temperatura usando la fórmula. */
      double next_temp = temperature + ((shared_data->delta_t *
        shared_data->alpha) / (shared_data->h * shared_data->h)) *
          (shared_data->data[i - 1][j] + shared_data->data[i + 1][j] +
            shared_data->data[i][j - 1] + shared_data->data[i][j + 1] - 4 *
              temperature);
      /** Guardar el valor calculado en la copia temporal de la matriz. */
      temp_data[i][j] = next_temp;
    }
  }

  /** Copiar los resultados de la matriz temporal a la matriz compartida. */
  for (uint64_t i = private_data->start_row; i < private_data->end_row; i++) {
    for (uint64_t j = 1; j < shared_data->cols - 1; j++) {
      shared_data->data[i][j] = temp_data[i][j];
    }
  }
  /** Liberar la memoria de la copia de la matriz. */
  for (uint64_t i = 0; i < shared_data->rows; i++) {
    free(temp_data[i]);
  }
  free(temp_data);
  return NULL;  /** Finalizar la ejecución del hilo. */
}
