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
  int rank, size;
  MPI_Comm_rank(MPI_COMM_WORLD, &rank);
  MPI_Comm_size(MPI_COMM_WORLD, &size);

  uint64_t rows, cols;
  double **local_data = NULL;
  uint64_t local_rows;

  if (rank == 0) {
    // Root process reads the input file.
    char bin_path[257];
    snprintf(bin_path, sizeof(bin_path), "%s/%s", input_dir, plate_filename);
    FILE* bin_file = fopen(bin_path, "rb");
    if (!bin_file) {
      fprintf(stderr, "Error opening binary file.\n");
      MPI_Abort(MPI_COMM_WORLD, 1);
    }

    fread(&rows, sizeof(uint64_t), 1, bin_file);
    fread(&cols, sizeof(uint64_t), 1, bin_file);

    // Calculate local rows for each process.
    local_rows = (rows / size) + 2;  ///< +2 for ghost rows.
    if (rank == size - 1) {
      local_rows = rows - (size - 1) * (rows / size) + 2;
    }

    // Broadcast dimensions to all processes.
    MPI_Bcast(&cols, 1, MPI_UNSIGNED_LONG_LONG, 0, MPI_COMM_WORLD);

    // Allocate and read data.
    double** data = (double**) malloc(rows * sizeof(double*));
    for (uint64_t i = 0; i < rows; i++) {
      data[i] = (double*) malloc(cols * sizeof(double));
      fread(data[i], sizeof(double), cols, bin_file);
    }
    fclose(bin_file);

    // Distribute data to other processes.
    for (int p = 1; p < size; p++) {
      uint64_t p_rows = (rows / size) + 2;
      if (p == size - 1) {
        p_rows = rows - (size - 1) * (rows / size) + 2;
      }

      for (uint64_t i = 0; i < p_rows - 2; i++) {
        MPI_Send(data[p * (rows / size) + i], cols, MPI_DOUBLE, p, 0,
          MPI_COMM_WORLD);
      }
    }

    // Keep root process's portion.
    local_data = (double**) malloc(local_rows * sizeof(double*));
    for (uint64_t i = 0; i < local_rows; i++) {
      local_data[i] = (double*) malloc(cols * sizeof(double));
      if (i > 0 && i < local_rows - 1) {
        memcpy(local_data[i], data[i-1], cols * sizeof(double));
      }
    }

    // Clean up original data.
    for (uint64_t i = 0; i < rows; i++) {
      free(data[i]);
    }
    free(data);
  } else {
    // Other processes receive their portion of data.
    MPI_Bcast(&cols, 1, MPI_UNSIGNED_LONG_LONG, 0, MPI_COMM_WORLD);

    local_rows = (rows / size) + 2;
    if (rank == size - 1) {
      local_rows = rows - (size - 1) * (rows / size) + 2;
    }

    local_data = (double**) malloc(local_rows * sizeof(double*));
    for (uint64_t i = 0; i < local_rows; i++) {
      local_data[i] = (double*) malloc(cols * sizeof(double));
      if (i > 0 && i < local_rows - 1) {
        MPI_Recv(local_data[i], cols, MPI_DOUBLE, 0, 0, MPI_COMM_WORLD,
          MPI_STATUS_IGNORE);
      }
    }
  }

  // Run simulation.
  uint64_t states = 0;
  simulate(local_data, local_rows, cols, params, &states, rank, size);

  // Gather results back to root process.
  if (rank == 0) {
    // Process timing and create output.
    const time_t secs = states * params.delta_t;
    char time[49];
    format_time(secs, time, sizeof(time));

    // Allocate memory for final data.
    double** final_data = (double**) malloc(rows * sizeof(double*));
    for (uint64_t i = 0; i < rows; i++) {
      final_data[i] = (double*) malloc(cols * sizeof(double));
    }

    // Copy root process data.
    for (uint64_t i = 0; i < local_rows - 2; i++) {
      memcpy(final_data[i], local_data[i+1], cols * sizeof(double));
    }

    // Receive data from other processes.
    for (int p = 1; p < size; p++) {
      uint64_t p_rows = (rows / size);
      if (p == size - 1) {
        p_rows = rows - (size - 1) * (rows / size);
      }

      for (uint64_t i = 0; i < p_rows; i++) {
        MPI_Recv(final_data[p * (rows / size) + i], cols, MPI_DOUBLE, p, 1,
          MPI_COMM_WORLD, MPI_STATUS_IGNORE);
      }
    }

    // Write output and create report.
    write_plate(output_dir, final_data, rows, cols, states, plate_filename);
    create_report(filepath, states, time, params, plate_filename);

    // Clean up final data.
    for (uint64_t i = 0; i < rows; i++) {
      free(final_data[i]);
    }
    free(final_data);
  } else {
    // Send local results back to root.
    for (uint64_t i = 1; i < local_rows - 1; i++) {
      MPI_Send(local_data[i], cols, MPI_DOUBLE, 0, 1, MPI_COMM_WORLD);
    }
  }

  // Clean up local data.
  for (uint64_t i = 0; i < local_rows; i++) {
    free(local_data[i]);
  }
  free(local_data);
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
