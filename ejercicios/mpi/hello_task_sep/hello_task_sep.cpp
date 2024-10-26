// Copyright 2024 Josué Torres Sibaja <josue.torressibaja@ucr.ac.cr>

/**
 * @file hello_task_sep.cpp
 * @brief MPI program that prints a greeting from process 0 and performs a
 * count from process 1, requiring exactly 2 processes.
 */

#include <mpi.h>
#include <iostream>

/**
 * @brief Main function that executes MPI operations.
 *
 * @details This program uses MPI to print a greeting from process 0 and
 * perform a count from 0 to 10 from process 1. It requires exactly 2 processes
 * to run.
 *
 * @param argc Number of command-line arguments.
 * @param argv Array of command-line arguments.
 * @return int Execution status code, 0 if successful, 1 if an error occurs.
 */
int main(int argc, char* argv[]) {
  /** Initialize MPI and check if initialization was successful. */
  if (MPI_Init(&argc, &argv) == MPI_SUCCESS) {
    int process_number = -1;  /** Current process number. */
    MPI_Comm_rank(MPI_COMM_WORLD, &process_number);

    int process_count = -1;  /** Total number of processes. */
    MPI_Comm_size(MPI_COMM_WORLD, &process_count);

    /** Check if the number of processes is exactly 2. */
    if (process_count != 2) {
      if (process_number == 0) {
        std::cerr << "Error: Must introduce exactly two processes."
          << std::endl;
      }
      MPI_Finalize();
      return 1;
    }

    /** Hostname where the process is running. */
    char process_hostname[MPI_MAX_PROCESSOR_NAME] = { '\0' };
    int hostname_length = -1;  /** Length of the hostname. */
    MPI_Get_processor_name(process_hostname, &hostname_length);

    /** Greeting from process 0. */
    if (process_number == 0) {
      std::cout << "Hello world from process " << process_number
        << " of " << process_count << " on " << process_hostname << std::endl;
    }

    /** Synchronize processes to ensure execution order. */
    MPI_Barrier(MPI_COMM_WORLD);

    /** Count from process 1. */
    if (process_number == 1) {
      std::cout << "Initializing count from process " << process_number
        << "..." << std::endl;
      for (int counter = 0; counter <= 10; counter++) {
        std::cout << counter << std::endl;
      }
    }

    MPI_Finalize();  /** Properly finalize MPI. */
  } else {
    std::cerr << "Error: Could not initialize MPI." << std::endl;
  }
  return 0;
}
