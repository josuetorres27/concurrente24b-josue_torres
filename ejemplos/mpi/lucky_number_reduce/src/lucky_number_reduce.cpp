// Copyright 2024 ECCI-UCR CC-BY 4.0
// Commented by Josue Torres Sibaja <josue.torressibaja@ucr.ac.cr>

/**
 * @file main.cpp
 * @brief MPI-based program to generate and aggregate "lucky number" statistics
 * for each process.
 *
 * @details This program initializes an MPI environment and assigns a random
 * "lucky number" to each process. Using MPI reduction operations, it
 * calculates and displays the minimum, maximum, and average lucky numbers
 * among all processes.
 *
 * @note The program can be compiled with (in case Makefile or other
 * compilation commands do not work):
 *
 *   mpic++ -o lucky_number_reduce lucky_number_reduce.cpp UniformRandom.cpp
 *   mpirun -np 4 ./lucky_number_reduce
 */

#include <mpi.h>
#include <iostream>
#include <stdexcept>
#include "UniformRandom.hpp"  // NOLINT

#define fail(msg) throw std::runtime_error(msg)

/**
 * @brief Generates and prints statistical data of "lucky numbers" across all
 * processes.
 *
 * @details Each process generates a "lucky number" and then participates in
 * reduction operations to compute the global minimum, maximum, and sum of
 * these numbers across all processes.
 *
 * @param process_number The rank of the calling process within MPI_COMM_WORLD.
 * @param process_count The total number of processes in MPI_COMM_WORLD.
 */
void generate_lucky_statistics(int process_number, int process_count);

/**
 * @brief Main entry point of the MPI program.
 *
 * Initializes the MPI environment, retrieves the process rank and count, then
 * calls generate_lucky_statistics to calculate and print lucky number
 * statistics. Catches and reports any runtime errors.
 *
 * @param argc Number of command-line arguments.
 * @param argv Array of command-line arguments.
 * @return int Returns EXIT_SUCCESS if successful; otherwise, EXIT_FAILURE.
 */
int main(int argc, char* argv[]) {
  int error = EXIT_SUCCESS;
  if (MPI_Init(&argc, &argv) == MPI_SUCCESS) {
    try {
      int process_number = -1;
      MPI_Comm_rank(MPI_COMM_WORLD, &process_number);

      int process_count = -1;
      MPI_Comm_size(MPI_COMM_WORLD, &process_count);

      char process_hostname[MPI_MAX_PROCESSOR_NAME] = { '\0' };
      int hostname_length = -1;
      MPI_Get_processor_name(process_hostname, &hostname_length);

      generate_lucky_statistics(process_number, process_count);
    } catch (const std::runtime_error& exception) {
      std::cout << exception.what() << std::endl;
      error = EXIT_FAILURE;
    }
    MPI_Finalize();
  } else {
    std::cerr << "error: could not init MPI" << std::endl;
    error = EXIT_FAILURE;
  }
  return error;
}

void generate_lucky_statistics(int process_number, int process_count) {
  // Generate "lucky number" for this process.
  UniformRandom<int> uniformRandom(process_number);
  const int my_lucky_number = uniformRandom.between(0, 100);

  std::cout << "Process " << process_number << ": my lucky number is "
    << my_lucky_number << std::endl;

  int all_min = -1;
  int all_max = -1;
  int all_sum = -1;

  // Reduce operations for minimum, maximum, and sum of lucky numbers.
  if (MPI_Reduce(&my_lucky_number, &all_min, 1, MPI_INT, MPI_MIN, 0,
    MPI_COMM_WORLD) != MPI_SUCCESS) {
    fail("Error: could not reduce min");
  }
  if (MPI_Reduce(&my_lucky_number, &all_max, 1, MPI_INT, MPI_MAX, 0,
    MPI_COMM_WORLD) != MPI_SUCCESS) {
    fail("Error: could not reduce max");
  }
  if (MPI_Reduce(&my_lucky_number, &all_sum, 1, MPI_INT, MPI_SUM, 0,
    MPI_COMM_WORLD) != MPI_SUCCESS) {
    fail("Error: could not reduce sum");
  }

  // Root process outputs the aggregated statistics.
  if (process_number == 0) {
    const double all_average = static_cast<double>(all_sum) / process_count;
    std::cout << "Process " << process_number << ": all minimum = " << all_min
      << std::endl;
    std::cout << "Process " << process_number << ": all average = "
      << all_average << std::endl;
    std::cout << "Process " << process_number << ": all maximum = " << all_max
      << std::endl;
  }
}
