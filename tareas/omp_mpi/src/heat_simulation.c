// Copyright 2024 Josue Torres Sibaja <josue.torressibaja@ucr.ac.cr>

#include "heat_simulation.h"

/**
 * @brief Configures and initiates a heat diffusion simulation from a binary
 * plate file.
 *
 * @details This function loads matrix data from a binary file, initializes
 * simulation parameters, and begins the heat diffusion process using the
 * 'simulate' function. The simulation is executed with a specified number of
 * threads, utilizing OpenMP to set the thread count according to system
 * capabilities.
 *
 * @param plate_filename The name of the binary file containing the plate's
 * initial state.
 * @param params A SimData structure containing thermal properties and
 * simulation parameters.
 * @param report_file The path to the output file where simulation results will
 * be written.
 * @param input_dir The directory where the binary input file is located.
 * @param thread_count Number of threads to use in the simulation; adjusted to
 * row count if needed.
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
  shared_data->delta = params.delta;
  shared_data->alpha = params.alpha;
  shared_data->h = params.h;
  shared_data->epsilon = params.epsilon;

  // Start simulation.
  uint64_t states = 0;
  simulate(&states, shared_data);

  // Calculate elapsed time.
  const time_t seconds = states * params.delta;
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
 * @brief Simulates heat diffusion in a matrix with OpenMP parallelism.
 *
 * @details This function iteratively calculates heat distribution in a matrix
 * until equilibrium is achieved, using OpenMP for parallel processing to
 * improve speed.
 * The function uses OpenMP to parallelize both matrix copying and heat
 * calculations, optimizing for faster execution. Memory for the matrix copy is
 * freed upon completion.
 *
 * @param states Pointer to store the number of iterations required to reach
 * equilibrium.
 * @param shared_data Pointer to a SharedData structure containing matrix data,
 * dimensions, and thermal properties for the simulation.
 */
void simulate(uint64_t* states, SharedData* shared_data) {
  // Allocate memory for matrix copy.
  double** matrix_copy = (double**)
    malloc(shared_data->rows * sizeof(double*));
  for (uint64_t i = 0; i < shared_data->rows; i++) {
    matrix_copy[i] = (double*) malloc(shared_data->cols * sizeof(double));
  }

  uint64_t state = 0;
  bool equilibrium = false;
  const double delta = shared_data->delta;
  const double h = shared_data->h;
  const double alpha = shared_data->alpha;
  double** matrix = shared_data->matrix;
  const double epsilon = shared_data->epsilon;

  while (!equilibrium) {
    state++;

    // Copy matrix.
    #pragma omp parallel for collapse(2)
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
          double new_temp = cell + (delta * alpha / (h * h)) *
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
