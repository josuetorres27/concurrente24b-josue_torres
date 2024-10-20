// Copyright 2024 Josué Torres Sibaja <josue.torressibaja@ucr.ac.cr>

#include "heat_simulation.h"

/**
 * @brief Configura los parámetros de la simulación y ejecuta la propagación
 * del calor.
 *
 * @details Esta función inicializa las estructuras de datos compartidos, lee
 * una matriz desde un archivo binario, configura los parámetros de la
 * simulación y ejecuta la propagación del calor utilizando múltiples hilos.
 * También gestiona la memoria y guarda los resultados de la simulación en un
 * archivo y un reporte.
 *
 * @param plate_filename Nombre del archivo binario que contiene los datos
 * iniciales de la lámina.
 * @param params Parámetros de la simulación.
 * @param report_file Nombre del archivo de reporte donde se escribirán los
 * resultados de la simulación.
 * @param input_dir Ruta del directorio donde se encuentra el archivo binario.
 * @param thread_count Número de hilos a utilizar para el cálculo en paralelo.
 */
void configure_simulation(const char* plate_filename, SimData params,
  const char* report_file, const char* input_dir, uint64_t thread_count) {
  /** Crear la ruta hacia el archivo binario. */
  char bin_path[257];
  snprintf(bin_path, sizeof(bin_path), "%s/%s", input_dir, plate_filename);
  FILE* plate_file = fopen(bin_path, "rb");
  if (!plate_file) {
    fprintf(stderr, "Could not open binary file.\n");
    return;
  }

  /** Asignar datos compartidos. */
  SharedData* shared_data = (SharedData*) calloc(1, sizeof(SharedData));
  assert(shared_data);

  /** Leer la cantidad de filas y columnas. */
  if (fread(&(shared_data->rows), sizeof(uint64_t), 1, plate_file) != 1) {
    fprintf(stderr, "Error reading the number of rows.\n");
    fclose(plate_file);
    free(shared_data);
    return;
  }
  if (fread(&(shared_data->cols), sizeof(uint64_t), 1, plate_file) != 1) {
    fprintf(stderr, "Error reading the number of columns.\n");
    fclose(plate_file);
    free(shared_data);
    return;
  }

  /** Ajustar el número de hilos si es mayor que el número de filas. */
  if (thread_count > shared_data->rows) {
    thread_count = shared_data->rows;
  }

  /** Asignación dinámica de memoria para la matriz. */
  shared_data->matrix = malloc(shared_data->rows * sizeof(double*));
  if (!shared_data->matrix) {
    fprintf(stderr, "Could not allocate memory for matrix rows.\n");
    fclose(plate_file);
    free(shared_data);
    return;
  }
  for (uint64_t i = 0; i < shared_data->rows; i++) {
    shared_data->matrix[i] = (double*) malloc(shared_data->cols * sizeof(double));
    if (!shared_data->matrix[i]) {
      fprintf(stderr, "Could not allocate memory for row %lu.\n", i);
      for (uint64_t j = 0; j < i; j++) {
        free(shared_data->matrix[j]);
      }
      free(shared_data->matrix);
      fclose(plate_file);
      free(shared_data);
      return;
    }
  }

  /** Rellenar la matriz con las temperaturas. */
  for (uint64_t i = 0; i < shared_data->rows; i++) {
    for (uint64_t j = 0; j < shared_data->cols; j++) {
      if (fread(&(shared_data->matrix[i][j]), sizeof(double), 1, plate_file) != 1) {
        fprintf(stderr, "Error reading matrix data at row %" PRIu64 ", \
          col %" PRIu64 ".\n", i, j);
        for (uint64_t k = 0; k < shared_data->rows; k++) {
          free(shared_data->matrix[k]);
        }
        free(shared_data->matrix);
        fclose(plate_file);
        free(shared_data);
        return;
      }
    }
  }
  fclose(plate_file);

  /** Llenar los datos compartidos con los parámetros de la simulación. */
  shared_data->delta_t = params.delta_t;
  shared_data->alpha = params.alpha;
  shared_data->h = params.h;
  shared_data->epsilon = params.epsilon;

  /** Iniciar el mutex. */
  pthread_mutex_init(&shared_data->matrix_mutex, NULL);

  /** Iniciar la simulación. */
  uint64_t states = 0;
  simulate(&states, thread_count, shared_data);

  /** Destruir el mutex. */
  pthread_mutex_destroy(&shared_data->matrix_mutex);

  /** Calcular el tiempo transcurrido. */
  const time_t seconds = states * params.delta_t;
  char time[49];
  format_time(seconds, time, sizeof(time));

  /** Escribir los nuevos datos de la lámina. */
  write_plate(input_dir, shared_data->matrix, shared_data->rows,
    shared_data->cols, states, plate_filename);

  /** Escribir el reporte. */
  create_report(report_file, states, time, params, plate_filename);

  /** Liberar la memoria. */
  for (uint64_t i = 0; i < shared_data->rows; i++) {
    free(shared_data->matrix[i]);
  }
  free(shared_data->matrix);
  free(shared_data);
}

/**
 * @brief Simula la propagación del calor en la matriz utilizando múltiples
 * hilos.
 *
 * @details Esta función distribuye las filas de la matriz entre los hilos,
 * realiza el cálculo en paralelo y verifica si el sistema ha alcanzado el
 * equilibrio térmico.
 *
 * @param states Puntero a la variable que almacena el número de estados hasta
 * el equilibrio.
 * @param thread_count Número de hilos a utilizar.
 * @param shared_data Puntero a la estructura de datos compartidos.
 */
void simulate(uint64_t* states, uint64_t thread_count,
  SharedData* shared_data) {
  pthread_t* threads = (pthread_t*) malloc(thread_count * sizeof(pthread_t));
  assert(threads);

  ThreadData* thread_data = (ThreadData*) malloc(thread_count *
    sizeof(ThreadData));
  assert(thread_data);

  /** Asignar memoria para una copia de la matriz. */
  double** matrix_copy = (double**) malloc(shared_data->rows *
    sizeof(double*));
  for (uint64_t i = 0; i < shared_data->rows; i++) {
    matrix_copy[i] = (double*) malloc(shared_data->cols * sizeof(double));
  }

  uint64_t state = 0;
  bool equilibrium = false;

  while (!equilibrium) {
    equilibrium = true;
    state++;

    /** Copiar la matriz antes de que los hilos la actualicen. */
    for (uint64_t i = 0; i < shared_data->rows; i++) {
      for (uint64_t j = 0; j < shared_data->cols; j++) {
        matrix_copy[i][j] = shared_data->matrix[i][j];
      }
    }

    /** Distribuir el trabajo entre los hilos. */
    for (uint64_t i = 0; i < thread_count; i++) {
      uint64_t rows_per_thread = (shared_data->rows - 2) / thread_count;
      thread_data[i].start_row = 1 + i * rows_per_thread;
      thread_data[i].end_row = (i == thread_count - 1) ?
        shared_data->rows - 1 : thread_data[i].start_row + rows_per_thread;
      thread_data[i].shared_data = shared_data;

      /** Crear los hilos. */
      pthread_create(&threads[i], NULL, thread_sim, &thread_data[i]);
    }

    /** Esperar que todos los hilos terminen. */
    for (uint64_t i = 0; i < thread_count; i++) {
      pthread_join(threads[i], NULL);
    }

    /** Verificar el equilibrio térmico. */
    for (uint64_t i = 1; i < shared_data->rows - 1; i++) {
      for (uint64_t j = 1; j < shared_data->cols - 1; j++) {
        double cell = shared_data->matrix[i][j];
        if (fabs(matrix_copy[i][j] - cell) >= shared_data->epsilon) {
          equilibrium = false;
          break;
        }
      }
      if (!equilibrium) {
        break;
      }
    }
  }

  *states = state;

  /** Liberar memoria. */
  for (uint64_t i = 0; i < shared_data->rows; i++) {
    free(matrix_copy[i]);
  }
  free(matrix_copy);
  free(threads);
  free(thread_data);
}

/**
 * @brief Función que ejecuta cada hilo para simular la propagación del calor
 * en un rango de filas de la matriz.
 *
 * @details Cada hilo es responsable de actualizar una porción de la matriz
 * basada en la ecuación de propagación del calor. Los valores actualizados se
 * escriben de vuelta en la matriz compartida.
 *
 * @param data Puntero a los datos privados del hilo, que contiene su rango de
 * filas y los datos compartidos.
 * @return NULL
 */
void* thread_sim(void* data) {
  assert(data);
  ThreadData* thread_data = (ThreadData*) data;
  SharedData* shared_data = thread_data->shared_data;

  uint64_t delta_t = shared_data->delta_t;
  uint64_t h = shared_data->h;
  double alpha = shared_data->alpha;
  double** matrix = shared_data->matrix;

  /** Crear una copia de la matriz en la memoria local del hilo. */
  double** thread_matrix = malloc(shared_data->rows * sizeof(double*));
  for (uint64_t i = 0; i < shared_data->rows; i++) {
    thread_matrix[i] = malloc(shared_data->cols * sizeof(double));
  }

  /** Realizar la simulación con las filas correspondientes. */
  for (uint64_t i = thread_data->start_row; i < thread_data->end_row; i++) {
    for (uint64_t j = 1; j < shared_data->cols - 1; j++) {
      double cell = matrix[i][j];
      double cells_around = matrix[i-1][j] + matrix[i][j+1] + matrix[i+1][j] +
        matrix[i][j-1];
      thread_matrix[i][j] = cell + (delta_t * alpha / (h * h)) *
        (cells_around - 4 * cell);
    }
  }

  /** Copiar los resultados en la matriz compartida. */
  for (uint64_t i = thread_data->start_row; i < thread_data->end_row; i++) {
    for (uint64_t j = 1; j < shared_data->cols - 1; j++) {
      shared_data->matrix[i][j] = thread_matrix[i][j];
    }
  }

  /** Liberar la memoria local. */
  for (uint64_t i = 0; i < shared_data->rows; i++) {
    free(thread_matrix[i]);
  }
  free(thread_matrix);

  return NULL;
}
