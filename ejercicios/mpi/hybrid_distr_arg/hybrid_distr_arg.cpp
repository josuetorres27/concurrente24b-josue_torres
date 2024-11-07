// Copyright 2023 Jeisson Hidalgo jeisson.hidalgo@ucr.ac.cr CC-BY-4
// Adapted by Josue Torres Sibaja C37853

#include <algorithm>
#include <cstdlib>
#include <iostream>
#include <mpi.h>
#include <omp.h>

/**
 * @brief Calculate the start index for a process's range.
 *
 * @param rank The rank of the current process.
 * @param end The upper limit of the overall range.
 * @param workers The total number of processes.
 * @param begin The lower limit of the overall range.
 * @return The calculated start index for the current process's range.
 */
int calculate_start(int rank, int end, int workers, int begin);

/**
 * @brief Calculate the finish index for a process's range.
 *
 * @param rank The rank of the current process.
 * @param end The upper limit of the overall range.
 * @param workers The total number of processes.
 * @param begin The lower limit of the overall range.
 * @return The calculated end index for the current process's range.
 */
int calculate_finish(int rank, int end, int workers, int begin);

/**
 * @brief Main function to divide a specified range of values among MPI
 * processes and OpenMP threads.
 *
 * @details This function initializes MPI and OpenMP environments, divides a
 * range of values among available MPI processes, and further divides the
 * sub-range of each process among OpenMP threads.
 *
 * @return int Returns 0 if executed successfully.
 */
int main(int argc, char* argv[]) {
  if (MPI_Init(&argc, &argv) == MPI_SUCCESS) {
    int rank, size;
    /** Get the rank of the current process. */
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    /** Get the total number of processes. */
    MPI_Comm_size(MPI_COMM_WORLD, &size);

    int overall_start = -1;
    int overall_finish = -1;

    /** Parse command-line arguments or input range from rank 0 process. */
    if (argc == 3) {
      overall_start = std::atoi(argv[1]);
      overall_finish = std::atoi(argv[2]);
    } else {
      if (rank == 0) {
        std::cin >> overall_start >> overall_finish;
        for (int target = 1; target < size; ++target) {
          MPI_Send(&overall_start, 1, MPI_INT, target, 0, MPI_COMM_WORLD);
          MPI_Send(&overall_finish, 1, MPI_INT, target, 0, MPI_COMM_WORLD);
        }
      } else {
        MPI_Recv(&overall_start, 1, MPI_INT, 0, 0, MPI_COMM_WORLD,
          MPI_STATUS_IGNORE);
        MPI_Recv(&overall_finish, 1, MPI_INT, 0, 0, MPI_COMM_WORLD,
          MPI_STATUS_IGNORE);
      }
    }

    const int process_start = calculate_start(rank, overall_finish, size,
      overall_start);
    const int process_finish = calculate_finish(rank, overall_finish, size,
      overall_start);
    const int process_size = process_finish - process_start;

    /** Get the hostname of the processor. */
    char hostname[MPI_MAX_PROCESSOR_NAME];
    int hostname_len;
    MPI_Get_processor_name(hostname, &hostname_len);

    /** Print the assigned range for this process. */
    std::cout << hostname << ':' << rank
      << ": range [" << process_start << ", " << process_finish
        << "[ size " << process_size << std::endl;

    #pragma omp parallel default(none) \
      shared(process_start, process_finish, std::cout, rank, hostname)
    {
      int thread_start = -1;
      int thread_finish = -1;

      /** Distribute range among threads. */
      #pragma omp for schedule(static)
      for (int index = process_start; index < process_finish; ++index) {
        if (thread_start == -1) {
          thread_start = index;
        }
        thread_finish = index + 1;
      }

      const int thread_size = thread_finish - thread_start;

      /** Print range for this thread (critical section). */
      #pragma omp critical
      std::cout << '\t' << hostname << ':' << rank
        << '.' << omp_get_thread_num() << ": range [" << thread_start
          << ", " << thread_finish << "[ size " << thread_size << std::endl;
    }

    MPI_Finalize();
  } else {
    std::cerr << "Error: could not init MPI" << std::endl;
  }
  return 0;
}

int calculate_start(int rank, int end, int workers, int begin) {
  const int range = end - begin;
  return begin + rank * (range / workers) + std::min(rank, range % workers);
}

int calculate_finish(int rank, int end, int workers, int begin) {
  return calculate_start(rank + 1, end, workers, begin);
}
