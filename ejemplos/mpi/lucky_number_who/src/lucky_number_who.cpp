// Copyright 2024 ECCI-UCR CC-BY 4.0
// Commented by Josue Torres Sibaja <josue.torressibaja@ucr.ac.cr>

/**
 * @file lucky_number_who.cpp
 * @brief A program that generates distributed lucky statistics using MPI.
 *
 * @details This program initializes an MPI environment, assigns a random
 * lucky number to each process, and computes global statistics such as the
 * minimum, maximum, and average of these numbers across all processes. Results
 * are printed by each process.
 *
 * @note The program can be compiled with (in case Makefile or other
 * compilation commands do not work):
 *
 *   mpic++ -o lucky_number_who lucky_number_who.cpp
 *   mpirun -np 4 ./lucky_number_who
 */

#include <mpi.h>
#include <iostream>
#include <stdexcept>

#include "UniformRandom.hpp"

// Macro to throw a runtime error with a message.
#define fail(msg) throw std::runtime_error(msg)

/**
 * @brief Generates and computes distributed lucky number statistics.
 *
 * @details Each process generates a random lucky number and calculates
 * distributed statistics (minimum, maximum, sum, and average) across all
 * processes. Outputs the results for the current process.
 *
 * @param process_number The rank of the current MPI process.
 * @param process_count The total number of MPI processes.
 */
void generate_lucky_statistics(int process_number, int process_count);

/**
 * @brief Entry point of the program.
 *
 * @details Initializes the MPI environment, retrieves process-specific
 * information, and invokes the lucky number statistics computation. Finalizes
 * MPI before exiting.
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
  // Generate my lucky number.
  UniformRandom<int> uniformRandom(process_number);
  const int my_lucky_number = uniformRandom.between(0, 100);

  std::cout << "Process " << process_number << ": my lucky number is "
    << my_lucky_number << std::endl;

  int all_min = -1;
  int all_max = -1;
  int all_sum = -1;

  // Update distributed statistics from processes' lucky numbers.
  if (MPI_Allreduce(/*input*/ &my_lucky_number, /*output*/ &all_min,
    /*count*/ 1, MPI_INT, MPI_MIN, MPI_COMM_WORLD) != MPI_SUCCESS) {
    fail("error: could not reduce min");
  }
  if (MPI_Allreduce(/*input*/ &my_lucky_number, /*output*/ &all_max,
    /*count*/ 1, MPI_INT, MPI_MAX, MPI_COMM_WORLD) != MPI_SUCCESS) {
    fail("error: could not reduce max");
  }
  if (MPI_Allreduce(/*input*/ &my_lucky_number, /*output*/ &all_sum,
    /*count*/ 1, MPI_INT, MPI_SUM, MPI_COMM_WORLD) != MPI_SUCCESS) {
    fail("error: could not reduce sum");
  }

  const double all_average = double(all_sum) / process_count;  // NOLINT
  if (my_lucky_number == all_min) {
    std::cout << "Process " << process_number << ": my lucky number ("
      << my_lucky_number << ") is the minimum (" << all_min << ")"
        << std::endl;
  }

  if (my_lucky_number < all_average) {
    std::cout << "Process " << process_number << ": my lucky number ("
      << my_lucky_number << ") is less than the average (" << all_average
      << ")" << std::endl;
  } else if (my_lucky_number > all_average) {
    std::cout << "Process " << process_number << ": my lucky number ("
      << my_lucky_number << ") is greater than the average (" << all_average
      << ")" << std::endl;
  } else {
    std::cout << "Process " << process_number << ": my lucky number ("
      << my_lucky_number << ") is equal to the average (" << all_average
      << ")" << std::endl;
  }

  if (my_lucky_number == all_max) {
    std::cout << "Process " << process_number << ": my lucky number ("
      << my_lucky_number << ") is the maximum (" << all_max << ")"
        << std::endl;
  }
}
