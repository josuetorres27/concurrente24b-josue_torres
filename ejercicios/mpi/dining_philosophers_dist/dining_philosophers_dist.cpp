// Copyright 2024 Josue Torres Sibaja <josue.torressibaja@ucr.ac.cr>

/**
 * @file dining_philosophers_dist.cpp
 * @brief MPI-based solution for the Dining Philosophers Problem.
 *
 * @details This program simulates the Dining Philosophers Problem using MPI
 * where a waiter process manages access to shared resources (chopsticks).
 *
 * @note The program can be compiled with (in case Makefile or other
 * compilation commands do not work):
 *
 *   mpic++ -o dining_philosophers_dist dining_philosophers_dist.cpp
 *   mpirun -np 6 ./dining_philosophers_dist
 *
 */

#include <mpi.h>
#include <cstdio>
#include <cstdlib>
#include <unistd.h>  // NOLINT

#define PHILOSOPHERS 5  ///< Number of philosophers (and chopsticks).
#define WAITER 0        ///< Rank of the waiter process.
#define REQUEST 1       ///< Message type for requesting chopsticks.
#define RELEASE 2       ///< Message type for releasing chopsticks.

// Function prototypes.
void philosopher(int id);
void waiter();

/**
 * @brief Main function to initialize MPI and assign roles to processes.
 *
 * @param argc Number of command-line arguments.
 * @param argv Array of command-line arguments.
 * @return int Exit status.
 */
int main(int argc, char **argv) {
  int rank;

  MPI_Init(&argc, &argv);
  MPI_Comm_rank(MPI_COMM_WORLD, &rank);

  if (rank == WAITER) {
    waiter();  ///< The process with rank 0 acts as the waiter.
  } else {
    philosopher(rank);  ///< Other processes are philosophers.
  }

  MPI_Finalize();
  return 0;
}

/**
 * @brief Waiter function to manage chopstick allocation.
 *
 * @details The waiter listens for requests from philosophers to acquire or
 * release chopsticks and grants or denies access based on availability.
 */
void waiter() {
  // Array representing chopstick availability.
  int chopsticks[PHILOSOPHERS] = {1, 1, 1, 1, 1};
  int msg[2], source;

  while (1) {
    MPI_Status status;
    MPI_Recv(&msg, 2, MPI_INT, MPI_ANY_SOURCE, MPI_ANY_TAG, MPI_COMM_WORLD,
      &status);
    source = status.MPI_SOURCE;

    if (msg[0] == REQUEST) {  ///< Request for chopsticks.
      int left = (source + 1) % PHILOSOPHERS;
      int right = source;

      if (chopsticks[left] && chopsticks[right]) {
        chopsticks[left] = 0;
        chopsticks[right] = 0;
        ///< Allow eating.
        MPI_Send(&msg, 1, MPI_INT, source, 0, MPI_COMM_WORLD);
      } else {
        int deny = -1;
        // Deny eating.
        MPI_Send(&deny, 1, MPI_INT, source, 0, MPI_COMM_WORLD);
      }
    } else if (msg[0] == RELEASE) {  ///< Release of chopsticks.
      int left = (source + 1) % PHILOSOPHERS;
      int right = source;
      chopsticks[left] = 1;
      chopsticks[right] = 1;
    }
  }
}

/**
 * @brief Philosopher function simulating thinking, requesting, eating, and
 * releasing chopsticks.
 *
 * @param id The rank of the philosopher process.
 */
void philosopher(int id) {
  int msg[2];
  int response;

  while (1) {
    printf("Philosopher %d is thinking...\n", id);
    // Simulate thinking.
    sleep(rand() % 3 + 1);  // NOLINT

    // Request chopsticks.
    printf("Philosopher %d is trying to eat...\n", id);
    msg[0] = REQUEST;
    msg[1] = id;
    MPI_Send(&msg, 2, MPI_INT, WAITER, 0, MPI_COMM_WORLD);

    MPI_Recv(&response, 1, MPI_INT, WAITER, 0, MPI_COMM_WORLD,
      MPI_STATUS_IGNORE);

    if (response != -1) {
      printf("Philosopher %d is eating...\n", id);
      // Simulate eating.
      sleep(rand() % 2 + 1);  // NOLINT

      // Release chopsticks.
      printf("Philosopher %d has finished eating and is releasing the "
        "chopsticks.\n", id);
      msg[0] = RELEASE;
      MPI_Send(&msg, 2, MPI_INT, WAITER, 0, MPI_COMM_WORLD);
    } else {
      printf("Philosopher %d couldn't eat, going back to thinking.\n", id);
    }
  }
}
