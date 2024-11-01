// Copyright 2024 Esteban Solís <esteban.solisalfaro@ucr.ac.cr>
// Copyright 2024 Josué Torres <josue.torressibaja@ucr.ac.cr>

#include <mpi.h>
#include <cstdlib>
#include <iostream>
#include <stdexcept>

#define fail(msg) throw std::runtime_error(msg)

void calc_sum(int process_number, int process_count,
  const char* process_hostname);

/**
 * @brief Main function to initialize MPI, set up process information, and call
 * calc_sum.
 *
 * @param argc Argument count from the command line.
 * @param argv Argument vector from the command line.
 * @return EXIT_SUCCESS if execution completed successfully; EXIT_FAILURE
 * otherwise.
 */
int main(int argc, char* argv[]) {
  int error = EXIT_SUCCESS;

  /** Initialize the MPI environment and check for success. */
  if (MPI_Init(&argc, &argv) == MPI_SUCCESS) {
    int process_number = -1; /** Identifier of the current process. */
    MPI_Comm_rank(MPI_COMM_WORLD, &process_number);

    /** Total number of processes in the communicator. */
    int process_count = -1;
    MPI_Comm_size(MPI_COMM_WORLD, &process_count);

    /** Hostname of the current process. */
    char process_hostname[MPI_MAX_PROCESSOR_NAME] = { '\0' };
    int hostname_length = -1;
    MPI_Get_processor_name(process_hostname, &hostname_length);

    try {
      /** Execute the random sum calculation function. */
      calc_sum(process_number, process_count, process_hostname);
    } catch (const std::runtime_error& exception) {
      std::cerr << "error: " << exception.what() << std::endl;
      error = EXIT_FAILURE;
    }

    /** Finalize the MPI environment to clean up resources. */
    MPI_Finalize();
  } else {
    std::cerr << "Error: could not init MPI" << std::endl;
    error = EXIT_FAILURE;
  }
  return error;
}

/**
 * @brief Generates the random numbers and calculates the total sum in a
 * distributed MPI environment.
 *
 * @param process_number The unique identifier for each process.
 * @param process_count The total number of processes in the MPI communicator.
 * @param process_hostname The hostname of the machine where the process is
 * running.
 */
void calc_sum(int process_number, int process_count,
  const char* process_hostname) {
  /** Generate a unique random number between 0 and 99 in each process. */
  std::srand(static_cast<unsigned>(time(0)) + process_number);
  int random_num = std::rand() % 100;

  int sum = random_num;  /** Total sum. */
  int acc_sum = 0;  /** Accumulative sum. */

  /** Print each process's assigned random number. */
  std::cout << "Process " << process_number << " on " << process_hostname
    << " has been assigned random number " << random_num << std::endl;

  /** Ring sum passing the accumulated value in a single direction. */
  for (int step = 0; step < process_count - 1; ++step) {
    int next_process = (process_number + 1) % process_count;
    int prev_process = (process_count + process_number - 1) % process_count;

    /** Send the accumulated sum to the next process. */
    MPI_Send(&sum, 1, MPI_INT, next_process, 0, MPI_COMM_WORLD);

    /** Receive the accumulated sum from the previous process. */
    MPI_Recv(&acc_sum, 1, MPI_INT, prev_process, 0, MPI_COMM_WORLD,
      MPI_STATUS_IGNORE);

    /** Update the total sum with the received value. */
    sum = acc_sum + random_num;
  }

  /** Print the total sum from each process. */
  std::cout << "From process " << process_number << " on " << process_hostname
    << ", the total sum is " << sum << std::endl;
}
