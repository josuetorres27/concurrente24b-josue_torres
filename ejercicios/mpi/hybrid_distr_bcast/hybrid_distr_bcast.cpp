// Copyright 2024 ECCI-UCR CC-BY-4
// Adapted by Josue Torres Sibaja C37853

#include <omp.h>
#include <algorithm>
#include <cstdlib>
#include <iostream>
#include <mpi.h>

/**
 * @brief Calculates the starting point of the work range for a specific
 * process.
 *
 * @param rank The identifier of the process within the MPI communicator.
 * @param end The upper limit of the general work range.
 * @param workers The total number of processes performing the work.
 * @param begin The lower limit of the general work range.
 * @return The starting point of the work range for the specified process.
 */
int calculate_start(int rank, int end, int workers, int begin) {
  const int range = end - begin;
  return begin + rank * (range / workers) + std::min(rank, range % workers);
}

/**
 * @brief Calculates the end point of the work range for a specific process.
 *
 * @param rank The identifier of the process within the MPI communicator.
 * @param end The upper limit of the general work range.
 * @param workers The total number of processes performing the work.
 * @param begin The lower limit of the general work range.
 * @return The end point of the work range for the specified process.
 */
int calculate_finish(int rank, int end, int workers, int begin) {
  return calculate_start(rank + 1, end, workers, begin);
}

/**
 * @brief Main entry point of the program.
 *
 * @details The program uses MPI to divide a general work range among processes
 * and OpenMP to divide each process's range among multiple threads.
 *
 * @note The program can be compiled with (in case Makefile or other
 * compilation commands do not work):
 *
 *   mpic++ -fopenmp -o hybrid_distr_bcast hybrid_distr_bcast.cpp
 *   mpirun -np 4 ./hybrid_distr_bcast 0 100
 *
 */
int main(int argc, char* argv[]) {
  MPI_Init(&argc, &argv);

  int rank, size;
  MPI_Comm_rank(MPI_COMM_WORLD, &rank);
  MPI_Comm_size(MPI_COMM_WORLD, &size);

  int overall_start = -1;
  int overall_finish = -1;

  if (rank == 0) {
    // The master process reads the input range.
    if (argc == 3) {
      overall_start = std::atoi(argv[1]);
      overall_finish = std::atoi(argv[2]);
    } else {
      std::cin >> overall_start >> overall_finish;
    }
  }

  // Collective communication to distribute the range to all processes.
  MPI_Bcast(&overall_start, 1, MPI_INT, 0, MPI_COMM_WORLD);
  MPI_Bcast(&overall_finish, 1, MPI_INT, 0, MPI_COMM_WORLD);

  const int process_start = calculate_start(rank, overall_finish, size,
    overall_start);
  const int process_finish = calculate_finish(rank, overall_finish, size,
    overall_start);
  const int process_size = process_finish - process_start;

  char hostname[MPI_MAX_PROCESSOR_NAME];
  int hostname_len;
  MPI_Get_processor_name(hostname, &hostname_len);

  std::cout << hostname << ':' << rank << ": range [" << process_start << ", "
    << process_finish << "] size " << process_size << std::endl;

  #pragma omp parallel default(none) \
    shared(process_start, process_finish, std::cout, rank, hostname)
  {
    int thread_start = -1;
    int thread_finish = -1;

    #pragma omp for schedule(static)
    for (int index = process_start; index < process_finish; ++index) {
      if (thread_start == -1) {
        thread_start = index;
      }
      thread_finish = index + 1;
    }

    const int thread_size = thread_finish - thread_start;
    #pragma omp critical
    std::cout << '\t' << hostname << ':' << rank << '.' << omp_get_thread_num()
      << ": range [" << thread_start << ", " << thread_finish << "] size "
        << thread_size << std::endl;
  }

  MPI_Finalize();
  return 0;
}
