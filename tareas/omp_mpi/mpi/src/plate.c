// Copyright 2024 Josue Torres Sibaja <josue.torressibaja@ucr.ac.cr>

#include "plate.h"  // NOLINT

/**
 * @brief Simulates heat transfer across a plate until equilibrium is reached.
 *
 * @param shared_data Pointer to a structure containing the simulation
 * parameters and matrices.
 * @param rank The MPI rank of the current process.
 * @param size The total number of MPI processes.
 * @return The total number of simulation steps executed.
 */
uint64_t simulate(SharedData* shared_data, int rank, int size) {
  // Initialize the temporary matrix for pointer swapping.
  for (uint64_t i = 0; i < shared_data->rows; i++) {
    for (uint64_t j = 0; j < shared_data->cols; j++) {
      shared_data->temp_matrix[i][j] = shared_data->matrix[i][j];
    }
  }

  uint64_t total_sim_states = 0;
  bool global_eq_point = false;  // Global equilibrium flag.
  bool local_eq_point;           // Local equilibrium flag.

  // Broadcast shared parameters to all MPI processes.
  MPI_Bcast(&shared_data->rows, 1, MPI_UINT64_T, 0, MPI_COMM_WORLD);
  MPI_Bcast(&shared_data->cols, 1, MPI_UINT64_T, 0, MPI_COMM_WORLD);
  MPI_Bcast(&shared_data->delta, 1, MPI_DOUBLE, 0, MPI_COMM_WORLD);
  MPI_Bcast(&shared_data->alpha, 1, MPI_DOUBLE, 0, MPI_COMM_WORLD);
  MPI_Bcast(&shared_data->h, 1, MPI_DOUBLE, 0, MPI_COMM_WORLD);
  MPI_Bcast(&shared_data->epsilon, 1, MPI_DOUBLE, 0, MPI_COMM_WORLD);
  MPI_Bcast(&shared_data->alpha_delta, 1, MPI_DOUBLE, 0, MPI_COMM_WORLD);

  // Determine the range of rows handled by each process.
  uint64_t rows_per_process = (shared_data->rows - 2) / size;
  uint64_t start_row = rank * rows_per_process + 1;
  uint64_t final_row = (rank == size - 1) ? shared_data->rows - 1 : start_row +
    rows_per_process;

  while (!global_eq_point) {
    total_sim_states++;
    local_eq_point = true;

    // Perform simulation step for the local rows.
    for (uint64_t i = start_row; i < final_row; i++) {
      for (uint64_t j = 1; j < shared_data->cols - 1; j++) {
        double current_temperature = shared_data->matrix[i][j];
        double surroundings_temperature = shared_data->matrix[i - 1][j] +
          shared_data->matrix[i + 1][j] + shared_data->matrix[i][j - 1] +
            shared_data->matrix[i][j + 1];
        double new_temperature = current_temperature +
          shared_data->alpha_delta * (surroundings_temperature - 4 *
            current_temperature);
        shared_data->temp_matrix[i][j] = new_temperature;

        if (fabs(new_temperature - current_temperature) >=
          shared_data->epsilon) {
          local_eq_point = false;
        }
      }
    }

    // Exchange boundary rows with neighboring processes.
    if (rank > 0) {
      MPI_Send(shared_data->temp_matrix[start_row], shared_data->cols,
        MPI_DOUBLE, rank - 1, 0, MPI_COMM_WORLD);
      MPI_Recv(shared_data->temp_matrix[start_row - 1], shared_data->cols,
        MPI_DOUBLE, rank - 1, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
    }
    if (rank < size - 1) {
      MPI_Send(shared_data->temp_matrix[final_row - 1], shared_data->cols,
        MPI_DOUBLE, rank + 1, 0, MPI_COMM_WORLD);
      MPI_Recv(shared_data->temp_matrix[final_row], shared_data->cols,
        MPI_DOUBLE, rank + 1, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
    }

    // Check for global equilibrium across all processes.
    bool local_eq = local_eq_point;
    bool global_eq;
    MPI_Allreduce(&local_eq, &global_eq, 1, MPI_C_BOOL, MPI_LAND,
      MPI_COMM_WORLD);
    global_eq_point = global_eq;

    // Swap the matrices for the next iteration.
    double** temp = shared_data->matrix;
    shared_data->matrix = shared_data->temp_matrix;
    shared_data->temp_matrix = temp;
  }

  return total_sim_states;
}

/**
 * @brief Reads a binary plate file and initializes the simulation environment.
 *
 * @param dir Directory containing the binary files.
 * @param sim_params Array of simulation parameter structures.
 * @param lines Number of simulation entries to process.
 * @param job_name Name of the job file for reporting.
 * @param rank The MPI rank of the current process.
 * @param size The total number of MPI processes.
 */
void read_plate(const char* dir, SimData* sim_params, uint64_t lines,
  const char* job_name, int rank, int size) {
  FILE *bin_file;
  char file_path[512];
  SharedData* shared_data = malloc(sizeof(SharedData));
  if (!shared_data) {
    fprintf(stderr, "Failed to allocate memory for shared_data.\n");
    return;
  }

  uint64_t* sim_states_array = malloc(lines * sizeof(uint64_t));
  if (!sim_states_array) {
    fprintf(stderr, "Failed to allocate memory for simulation states.\n");
    free(shared_data);
    return;
  }

  for (uint64_t i = 0; i < lines; i++) {
    snprintf(file_path, sizeof(file_path), "%s/%s", dir,
      sim_params[i].bin_name);
    bin_file = fopen(file_path, "rb");
    if (!bin_file) {
      fprintf(stderr, "Failed to open binary file: %s.\n",
        sim_params[i].bin_name);
      free(sim_states_array);
      free(shared_data);
      return;
    }

    fread(&shared_data->rows, sizeof(uint64_t), 1, bin_file);
    fread(&shared_data->cols, sizeof(uint64_t), 1, bin_file);

    shared_data->matrix = create_matrix(shared_data->rows, shared_data->cols);
    shared_data->temp_matrix = create_matrix(shared_data->rows,
      shared_data->cols);
    if (!shared_data->matrix || !shared_data->temp_matrix) {
      fprintf(stderr, "Failed to allocate memory for matrices.\n");
      fclose(bin_file);
      free_matrix(shared_data->matrix, shared_data->rows);
      free_matrix(shared_data->temp_matrix, shared_data->rows);
      free(sim_states_array);
      free(shared_data);
      return;
    }

    for (uint64_t i = 0; i < shared_data->rows; i++) {
      if (fread(shared_data->matrix[i], sizeof(double), shared_data->cols,
        bin_file) != shared_data->cols) {
        fprintf(stderr, "Error reading matrix data from file: %s.\n",
          sim_params[i].bin_name);
        free_matrix(shared_data->matrix, shared_data->rows);
        free_matrix(shared_data->temp_matrix, shared_data->rows);
        fclose(bin_file);
        free(sim_states_array);
        free(shared_data);
        return;
      }
    }

    shared_data->delta = sim_params[i].delta;
    shared_data->alpha = sim_params[i].alpha;
    shared_data->h = sim_params[i].h;
    shared_data->epsilon = sim_params[i].epsilon;
    shared_data->alpha_delta = sim_params[i].delta * sim_params[i].alpha /
      (sim_params[i].h * sim_params[i].h);

    sim_states_array[i] = simulate(shared_data, rank, size);
    write_plate(shared_data->matrix, shared_data->rows, shared_data->cols, dir,
      sim_params[i].bin_name, sim_states_array[i]);

    free_matrix(shared_data->matrix, shared_data->rows);
    free_matrix(shared_data->temp_matrix, shared_data->rows);
    fclose(bin_file);
  }

  create_report(dir, job_name, sim_params, sim_states_array, lines);
  free(sim_states_array);
  free(shared_data);
}

/**
 * @brief Allocates memory for a matrix with the specified dimensions.
 *
 * @param rows Number of rows in the matrix.
 * @param cols Number of columns in the matrix.
 * @return Pointer to the created matrix.
 */
double** create_matrix(uint64_t rows, uint64_t cols) {
  double** matrix = malloc(rows * sizeof(double*));
  if (!matrix) {
    fprintf(stderr, "Failed to allocate memory for matrix rows.\n");
    return NULL;
  }

  for (uint64_t i = 0; i < rows; i++) {
    matrix[i] = malloc(cols * sizeof(double));
    if (!matrix[i]) {
      fprintf(stderr, "Failed to allocate memory for matrix columns.\n");
      for (uint64_t j = 0; j < i; j++) {
        free(matrix[j]);
      }
      free(matrix);
      return NULL;
    }
  }

  return matrix;
}

/**
 * @brief Frees the memory allocated for a matrix.
 *
 * @param matrix Pointer to the matrix to free.
 * @param rows Number of rows in the matrix.
 */
void free_matrix(double** matrix, uint64_t rows) {
  for (uint64_t i = 0; i < rows; i++) {
    free(matrix[i]);
  }
  free(matrix);
}
