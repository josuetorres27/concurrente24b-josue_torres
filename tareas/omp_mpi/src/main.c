// Copyright 2024 Josue Torres Sibaja <josue.torressibaja@ucr.ac.cr>

#include "heat_simulation.h"

/**
 * @file main.c
 * @brief Program to perform heat propagation simulations on a plate.
 *
 * @details The program reads the dimensions and initial temperature values of
 * a plate from a binary file, performs a heat propagation simulation to find
 * the moment of thermal equilibrium, and generates report and output files
 * with the results. The entered threads perform operations on a given number
 * of rows of the plate.
 */

/**
 * @brief Main function to run thermal simulations using multiple threads.
 *
 * @details The function takes as input a job file containing the
 * specifications of several thermal simulations to be performed. Each line in
 * the file contains information about a simulation job. The program can use a
 * user-specified number of threads or detect the number of available
 * processors to determine the number of threads to create. The number of
 * threads entered is used to perform operations on the array, distributing the
 * work equally among them.
 */
int main(int argc, char *argv[]) {
  double start_time = omp_get_wtime();  ///< OpenMP timing.

  // Verify command line arguments.
  if (argc < 4 || argc > 5) {
    /**
     * If you want to compile the program with the Makefile in the root
     * directory "omp_mpi", you must run the program as follows:
     * bin/omp_mpi job001.txt test/job001/input test/job001/output 4
     *
     * The "job" file can be replaced by the desired job number, as well as
     * the thread count (it will use as many threads as available CPUs if the
     * thread count is not provided).
     */
    fprintf(stderr, "Usage: bin/omp_mpi <job file> <input dir> <output dir> "
      "<thread_count>\n");
    return 11;
  }
  const char* job_filename = argv[1];
  const char* input_dir = argv[2];
  const char* output_dir = argv[3];

  // Configure thread count.
  uint64_t thread_count = sysconf(_SC_NPROCESSORS_ONLN);
  if (argc == 5) {
    if (sscanf(argv[4], "%" SCNu64, &thread_count) != 1) {
      fprintf(stderr, "Invalid thread count.\n");
      return 12;
    }
  }

  // Set OpenMP thread count.
  omp_set_num_threads(thread_count);

  // Extract job number from filename.
  uint64_t job_num = 0;
  sscanf(job_filename, "job%03lu.txt", &job_num);

  // Create paths for input/output files.
  char filepath[150];
  snprintf(filepath, sizeof(filepath), "%s", output_dir);
  char txt_path[255];
  snprintf(txt_path, sizeof(txt_path), "%s/%s", input_dir, job_filename);
  char report_path[255];
  snprintf(report_path, sizeof(report_path), "%s/job%03lu.tsv", filepath,
    job_num);

  // Create report file.
  FILE* report_file = fopen(report_path, "w");
  if (!report_file) {
    perror("Error opening report file.");
    return 1;
  }
  fclose(report_file);

  // Read simulation parameters.
  uint64_t struct_count = 0;
  SimData* simulation_parameters = read_job_file(txt_path, &struct_count);
  if (simulation_parameters == NULL) {
    fprintf(stderr, "Error reading job file.\n");
    return 1;
  }

  // Run simulation.
  const char* plate_filename;
  for (uint64_t i = 0; i < struct_count; i++) {
    plate_filename = simulation_parameters[i].bin_name;
    configure_simulation(plate_filename, simulation_parameters[i], report_path,
      input_dir, thread_count);
  }

  // Calculate elapsed time using OpenMP timing.
  double end_time = omp_get_wtime();
  double elapsed_secs = end_time - start_time;
  double elapsed_ns = elapsed_secs * 1e9;

  printf("Execution time (seconds): %.9lf\n", elapsed_secs);
  printf("Execution time (nanoseconds): %.9lf\n", elapsed_ns);

  // Free memory.
  free(simulation_parameters);
  return 0;
}
