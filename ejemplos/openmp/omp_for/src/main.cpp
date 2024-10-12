// Copyright 2024 ECCI-UCR CC-BY 4.0
#include <iostream>
#include <omp.h>  // NOLINT

/**
 * @brief Main function to execute parallel iterations using OpenMP.
 *
 * @details This program runs a specified number of iterations in parallel,
 * printing the current iteration number for each thread. The number of threads
 * and iterations can be provided as command-line arguments. If no arguments
 * are provided, it defaults to using the number of processors available and
 * the same number of iterations as threads.
 *
 * @param argc Number of command-line arguments.
 * @param argv Array of command-line argument strings.
 *
 * @return Exit status of the program.
 */
int main(int argc, char* argv[]) {
  /** Number of threads to be used. */
  const int thread_count = argc >= 2 ? ::atoi(argv[1]) : omp_get_num_procs();

  /** Number of iterations to be run. */
  const int iteration_count = argc >= 3 ? ::atoi(argv[2]) : thread_count;

  #pragma omp parallel for num_threads(thread_count) schedule(dynamic, 2) \
    default(none) shared(std::cout, thread_count, iteration_count)
  for (int iteration = 0; iteration < iteration_count; ++iteration) {
    #pragma omp critical
    std::cout << omp_get_thread_num() << '/' << thread_count
      << ": iteration " << iteration << '/' << iteration_count << std::endl;
  }
}
