// Copyright 2024 Josué Torres Sibaja <josue.torressibaja@ucr.ac.cr>

#include "plate.h"

/**
 * @brief Lee el archivo binario de la placa, crea los datos y llama a las
 * funciones para ejecutar la simulación y escribir los archivos de salida.
 *
 * @param plate_filename Nombre del archivo binario que contiene los datos.
 * @param params Estructura que contiene los parámetros de la simulación.
 * @param filepath Ruta donde se guardarán el reporte y los archivos de salida.
 * @param input_dir Directorio donde se encuentra el archivo binario.
 * @param output_dir Directorio donde se guardará el archivo binario de salida.
 */
void configure_simulation(const char* plate_filename, SimData params,
  const char* filepath, const char* input_dir, const char* output_dir) {
  /** Crear la ruta hacia el archivo binario. */
  char bin_path[257];
  snprintf(bin_path, sizeof(bin_path), "%s/%s", input_dir, plate_filename);
  FILE* bin_file;
  uint64_t rows, cols;
  bin_file = fopen(bin_path, "rb");
  if (!bin_file) {
    fprintf(stderr, "Error opening binary file.\n");
    return;
  }

  /** Leer los valores de la lámina. */
  if (fread(&rows, sizeof(uint64_t), 1, bin_file) != 1) {
    fprintf(stderr, "Error reading number of rows.\n");
    fclose(bin_file);
    return;
  }
  if (fread(&cols, sizeof(uint64_t), 1, bin_file) != 1) {
    fprintf(stderr, "Error reading number of columns.\n");
    fclose(bin_file);
    return;
  }

  double** data = (double**) malloc(rows * sizeof(double*));
  if (!data) {
    fprintf(stderr, "Error allocating memory for plate rows.\n");
    fclose(bin_file);
    return;
  }

  for (uint64_t i = 0; i < rows; i++) {
    data[i] = (double*) malloc(cols * sizeof(double));
    if (!data[i]) {
      fprintf(stderr, "Error allocating memory for plate columns.\n");
      for (uint64_t j = 0; j < i; j++) {
        free(data[j]);
      }
      free(data);
      fclose(bin_file);
      return;
    }
  }

  for (uint64_t i = 0; i < rows; i++) {
    for (uint64_t j = 0; j < cols; j++) {
      if (fread(&data[i][j], sizeof(double), 1, bin_file) != 1) {
        fprintf(stderr, "Error reading plate data.\n");
        for (uint64_t k = 0; k < rows; k++) {
          free(data[k]);
        }
        free(data);
        fclose(bin_file);
        return;
      }
    }
  }
  fclose(bin_file);

  /** Ejecutar la simulación. */
  uint64_t states = 0;
  simulate(data, rows, cols, params, &states);

  /** Tomar el tiempo transcurrido. */
  const time_t secs = states * params.delta_t;
  char time[49];
  format_time(secs, time, sizeof(time));

  /** Escribir la nueva placa. */
  write_plate(output_dir, data, rows, cols, states, plate_filename);
  for (uint64_t i = 0; i < rows; i++) {
    free(data[i]);
  }
  free(data);

  /** Escribir el reporte. */
  create_report(filepath, states, time, params, plate_filename);
}

/**
 * @brief Ejecuta la simulación térmica en cada lámina hasta que se alcance el
 * equilibrio térmico.
 *
 * @param data Datos que representan el estado inicial de la lámina.
 * @param rows Número de filas de la matriz.
 * @param cols Número de columnas de la matriz.
 * @param params Estructura que contiene los parámetros de la simulación.
 * @param states Puntero para almacenar el número de iteraciones necesarias
 * para alcanzar el equilibrio.
 */
void simulate(double** local_data, uint64_t local_rows, uint64_t cols,
  SimData params, uint64_t* states, int rank, int size) {
  MPI_Status status;
  const int top = (rank == 0) ? MPI_PROC_NULL : rank - 1;
  const int bottom = (rank == size - 1) ? MPI_PROC_NULL : rank + 1;

  // Allocate memory for ghost rows and computation buffer.
  double* top_ghost = (double*) malloc(cols * sizeof(double));
  double* bottom_ghost = (double*) malloc(cols * sizeof(double));
  double** local_copy = (double**) malloc(local_rows * sizeof(double*));

  for (uint64_t i = 0; i < local_rows; i++) {
    local_copy[i] = (double*) malloc(cols * sizeof(double));
    if (!local_copy[i]) {
      fprintf(stderr, "Error allocating memory for local copy.\n");
      MPI_Abort(MPI_COMM_WORLD, 1);
    }
  }

  uint64_t state = 0;
  double local_max_epsilon, global_max_epsilon;
  global_max_epsilon = params.epsilon + 1;

  // Main simulation loop.
  while (global_max_epsilon > params.epsilon) {
    local_max_epsilon = 0.0;
    state++;

    // Exchange ghost rows.
    MPI_Sendrecv(local_data[1], cols, MPI_DOUBLE, top, 0, bottom_ghost, cols,
      MPI_DOUBLE, bottom, 0, MPI_COMM_WORLD, &status);

    MPI_Sendrecv(local_data[local_rows-2], cols, MPI_DOUBLE, bottom, 1,
      top_ghost, cols, MPI_DOUBLE, top, 1, MPI_COMM_WORLD, &status);

    // Update interior points.
    for (uint64_t i = 1; i < local_rows - 1; i++) {
      for (uint64_t j = 1; j < cols - 1; j++) {
        double cell = local_data[i][j];
        double cells_around;

        // Handle boundary cases.
        if (i == 1 && rank > 0) {
          cells_around = top_ghost[j] + local_data[i][j+1] +
            local_data[i+1][j] + local_data[i][j-1];
        } else if (i == local_rows-2 && rank < size-1) {
          cells_around = local_data[i-1][j] + local_data[i][j+1] +
            bottom_ghost[j] + local_data[i][j-1];
        } else {
          cells_around = local_data[i-1][j] + local_data[i][j+1] +
            local_data[i+1][j] + local_data[i][j-1];
        }

        // Apply heat equation.
        local_copy[i][j] = cell + (params.delta_t * params.alpha /
          (params.h * params.h)) * (cells_around - 4 * cell);

        // Calculate maximum difference.
        double difference = fabs(local_copy[i][j] - cell);
        if (difference > local_max_epsilon) {
          local_max_epsilon = difference;
        }
      }
    }

    // Find global maximum epsilon across all processes.
    MPI_Allreduce(&local_max_epsilon, &global_max_epsilon, 1, MPI_DOUBLE,
      MPI_MAX, MPI_COMM_WORLD);

    // Copy updated values back to original array.
    for (uint64_t i = 1; i < local_rows - 1; i++) {
      for (uint64_t j = 1; j < cols - 1; j++) {
        local_data[i][j] = local_copy[i][j];
      }
    }
  }

  // Gather final number of iterations.
  MPI_Bcast(&state, 1, MPI_UNSIGNED_LONG_LONG, 0, MPI_COMM_WORLD);
  *states = state;

  // Clean up.
  free(top_ghost);
  free(bottom_ghost);
  for (uint64_t i = 0; i < local_rows; i++) {
    free(local_copy[i]);
  }
  free(local_copy);
}
