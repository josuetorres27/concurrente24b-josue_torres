// Copyright 2021 Jeisson Hidalgo-Cespedes <jeisson.hidalgo@ucr.ac.cr> CC-BY-4
// Simulates a producer and a consumer that share a unbounded buffer

/**
 * @file main.c
 * @brief Runs the simulation of the producer-consumer problem with an
 * unbounded buffer.
 * 
 * @details This program creates a producer and a consumer that share a buffer
 * of unlimited capacity. Both of them operate concurrently using threads.
 * The producer fills the buffer with data, and the consumer reads the data.
 * Delays can be configured for both the producer and consumer to simulate
 * real-world processing time. The program takes command-line arguments that
 * define the buffer capacity, the number of rounds to produce and consume
 * data, and the minimum and maximum delays for the producer and consumer.
 */

#include <stdlib.h>
#include <stdio.h>

#include "simulation.h"

/**
 * @brief Performs the simulation.
 * 
 * @details This function initializes the simulation by creating a simulation
 * object. If the object is created successfully, the function runs the
 * simulation and then destroys the simulation object. If the simulation object
 * could not be created, it displays an error message.
 * @param argc Number of command-line arguments passed to the program.
 * @param argv Array of strings representing the command-line arguments.
 * 
 * @return EXIT_SUCCESS if the program runs successfully, or EXIT_FAILURE
 * if there is an error.
 */
int main(int argc, char* argv[]) {
  int error = EXIT_SUCCESS;
  /** Create the simulation object. */
  simulation_t* simulation = simulation_create();
  if (simulation) {
    error = simulation_run(simulation, argc, argv);
    /** Destroy the simulation object to free memory. */
    simulation_destroy(simulation);
  } else {
    fprintf(stderr, "error: could not allocate simulation\n");
    error = EXIT_FAILURE;
  }
  return error;
}
