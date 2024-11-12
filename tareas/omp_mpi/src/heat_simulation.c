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
  // Create path to binary file.
  char bin_path[257];
  snprintf(bin_path, sizeof(bin_path), "%s/%s", input_dir, plate_filename);
  FILE* plate_file = fopen(bin_path, "rb");
  if (!plate_file) {
    fprintf(stderr, "Could not open binary file.\n");
    return;
  }

  // Allocate shared data.
  SharedData* shared_data = (SharedData*) calloc(1, sizeof(SharedData));
  assert(shared_data);

  // Read rows and columns.
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

  // Adjust thread count if greater than number of rows.
  if (thread_count > shared_data->rows) {
    thread_count = shared_data->rows;
  }
  omp_set_num_threads(thread_count);

  // Allocate memory for matrix.
  shared_data->matrix = malloc(shared_data->rows * sizeof(double*));
  if (!shared_data->matrix) {
    fprintf(stderr, "Could not allocate memory for matrix rows.\n");
    fclose(plate_file);
    free(shared_data);
    return;
  }
  for (uint64_t i = 0; i < shared_data->rows; i++) {
    shared_data->matrix[i] = (double*)
      malloc(shared_data->cols * sizeof(double));
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

  // Fill matrix with temperatures.
  for (uint64_t i = 0; i < shared_data->rows; i++) {
    for (uint64_t j = 0; j < shared_data->cols; j++) {
      if (fread(&(shared_data->matrix[i][j]), sizeof(double), 1,
        plate_file) != 1) {
        fprintf(stderr, "Error reading matrix data.\n");
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

  // Fill shared data with simulation parameters.
  shared_data->delta_t = params.delta_t;
  shared_data->alpha = params.alpha;
  shared_data->h = params.h;
  shared_data->epsilon = params.epsilon;

  // Start simulation.
  uint64_t states = 0;
  simulate(&states, thread_count, shared_data);

  // Calculate elapsed time.
  const time_t seconds = states * params.delta_t;
  char time[49];
  format_time(seconds, time, sizeof(time));

  // Write new plate data.
  write_plate(input_dir, shared_data->matrix, shared_data->rows,
    shared_data->cols, states, plate_filename);

  // Write report.
  create_report(report_file, states, time, params, plate_filename);

  // Free memory.
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
  // Allocate memory for matrix copy.
  double** matrix_copy = (double**)
    malloc(shared_data->rows * sizeof(double*));
  for (uint64_t i = 0; i < shared_data->rows; i++) {
    matrix_copy[i] = (double*) malloc(shared_data->cols * sizeof(double));
  }

  uint64_t state = 0;
  bool equilibrium = false;
  const double delta_t = shared_data->delta_t;
  const double h = shared_data->h;
  const double alpha = shared_data->alpha;
  double** matrix = shared_data->matrix;
  const double epsilon = shared_data->epsilon;

  while (!equilibrium) {
    state++;

    // Copy matrix.
    #pragma omp parallel for collapse(2) nowait
    for (uint64_t i = 0; i < shared_data->rows; i++) {
      for (uint64_t j = 0; j < shared_data->cols; j++) {
        matrix_copy[i][j] = matrix[i][j];
      }
    }

    // Ensure matrix copy is complete before calculations.
    #pragma omp barrier

    bool local_equilibrium = true;
    #pragma omp parallel
    {
      bool thread_equilibrium = true;

      #pragma omp for collapse(2) schedule(static)
      for (uint64_t i = 1; i < shared_data->rows - 1; i++) {
        for (uint64_t j = 1; j < shared_data->cols - 1; j++) {
          double cell = matrix_copy[i][j];
          double cells_around = matrix_copy[i-1][j] + matrix_copy[i][j+1] +
            matrix_copy[i+1][j] + matrix_copy[i][j-1];
          double new_temp = cell + (delta_t * alpha / (h * h)) *
            (cells_around - 4 * cell);
          matrix[i][j] = new_temp;

          if (fabs(new_temp - cell) >= epsilon) {
            thread_equilibrium = false;
          }
        }
      }

      // Critical section with minimal overhead.
      #pragma omp critical
      {
        if (!thread_equilibrium) local_equilibrium = false;
      }
    }

    equilibrium = local_equilibrium;
  }

  *states = state;

  // Free memory.
  for (uint64_t i = 0; i < shared_data->rows; i++) {
    free(matrix_copy[i]);
  }
  free(matrix_copy);
}
