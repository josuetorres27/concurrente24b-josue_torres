// Copyright 2024 Josue Torres Sibaja <josue.torressibaja@ucr.ac.cr>

#include "plate.h"  // NOLINT

/**
 * @brief Main function for the MPI-based heat transfer simulation program.
 *
 * @param argc Number of command-line arguments.
 * @param argv Array of command-line arguments.
 * argv[1]: Name of the job file (e.g., job001.txt).
 * argv[2]: Directory path where the job file is located (TSV and binary files
 * will be generated here as well).
 * @return Returns 0 if execution is successful, or -1 if an error occurs.
 */
int main(int argc, char *argv[]) {
  // Initialize the MPI environment.
  double total_start_time, total_end_time;
  int rank = -1;
  total_start_time = MPI_Wtime();

  if (MPI_Init(&argc, &argv) == MPI_SUCCESS) {
    int size = -1;  // Total number of processes.
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &size);

    // Ensure the necessary arguments are provided.
    if (argc < 3) {
      /**
       * @note The program can be compiled with (in case Makefile or other
       * compilation commands do not work):
       *
       * mpicc -o heat_simulation main.c plate.c utils.c -lm
       * mpirun --oversubscribe -n 4 ./heat_simulation job001.txt ../test/job001/input  // NOLINT
       *
       * The "job" file can be replaced by the desired job number, as well as
       * the process count.
       */
      fprintf(stderr, "Please run the program with: mpirun --oversubscribe -n "
        "<process_count> ./<executable file name> <job file name> "
          "<input directory>\n");
      MPI_Finalize();
      return -1;
    }

    // Parse command-line arguments.
    const char* job_name = argv[1];  // Job file name.
    const char* dir = argv[2];       // Directory containing the job file.

    if (argc == 3) {
      uint64_t lines = 0;
      double read_job_start_time, read_job_end_time;
      double read_plate_start_time, read_plate_end_time;

      // Read the job file and retrieve simulation parameters.
      read_job_start_time = MPI_Wtime();
      SimData* sim_params = read_job_file(job_name, dir, &lines);
      read_job_end_time = MPI_Wtime();

      // Process the simulation parameters using MPI.
      read_plate_start_time = MPI_Wtime();
      read_plate(dir, sim_params, lines, job_name, rank, size);
      read_plate_end_time = MPI_Wtime();

      // Free allocated memory for each SimData entry.
      for (uint64_t i = 0; i < lines; i++) {
        if (sim_params[i].bin_name != NULL) {
          free(sim_params[i].bin_name);
        }
      }

      // Free the array of SimData structs.
      free(sim_params);
    }

    // Finalize the MPI environment.
    MPI_Finalize();
  } else {
    // Error initializing MPI.
    fprintf(stderr, "Error: Failed to initialize MPI.\n");
  }

  total_end_time = MPI_Wtime();
  if (rank == 0) {
    printf("Total execution time: %f seconds\n",
      total_end_time - total_start_time);
  }

  return 0;
}
