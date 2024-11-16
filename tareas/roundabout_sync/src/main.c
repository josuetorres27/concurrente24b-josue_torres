// Copyright 2024 Josue Torres Sibaja <josue.torressibaja@ucr.ac.cr>

/**
 * @file main.c
 * @brief Simulates a roundabout traffic system.
 *
 * @details This program initializes a traffic simulation for a roundabout
 * system, allowing user input for vehicle movements and simulating the passage
 * of vehicles through the roundabout using threads.
 */

#include "roundabout.h"

/**
 * @brief Main function for the roundabout simulation.
 *
 * @details This function initializes the simulation state, takes user input
 * for configuration and vehicle data, and runs the simulation using
 * multithreading to simulate vehicle movement.
 *
 * @param argc The number of command-line arguments.
 * @param argv Command-line arguments:
 * - 'argv[1]': Minimum simulation time per segment.
 * - 'argv[2]': Maximum simulation time per segment.
 * - 'argv[3]' (optional): Verbose mode flag (-v).
 *
 * @return int Returns 0 on successful completion or 1 on error.
 */
int main(int argc, char* argv[]) {
  int min_time = 0, max_time = 0, verbose_mode = 0;

  // Parse command-line arguments.
  if (argc >= 3) {
    min_time = atoi(argv[1]);
    max_time = atoi(argv[2]);
  }
  if (argc == 4 && strcmp(argv[3], "-v") == 0) {
    verbose_mode = 1;
  }

  // Get segment capacity from the user.
  int segment_capacity;
  printf("Enter the capacity of each segment: ");
  if (scanf("%d", &segment_capacity) != 1 || segment_capacity <= 0) {
    fprintf(stderr, "Error: Capacity must be a positive integer.\n");
    return 1;
  }

  // Initialize the simulation state.
  SimulationState* sim_state = init_simulation(min_time, max_time,
    verbose_mode, segment_capacity);

  // Input vehicle data.
  printf("Enter vehicles (format: input output, e.g., NE):\n");
  char entry, exit;
  while (scanf(" %c %c", &entry, &exit) == 2 &&
    sim_state->num_vehicles < MAX_VEHICLES) {
    sim_state->vehicles[sim_state->num_vehicles].id = sim_state->num_vehicles;
    sim_state->vehicles[sim_state->num_vehicles].entry = entry;
    sim_state->vehicles[sim_state->num_vehicles].exit = exit;
    sim_state->num_vehicles++;
  }

  // Seed the random number generator.
  srand(time(NULL));

  // Create threads for each vehicle.
  pthread_t threads[MAX_VEHICLES];
  for (int i = 0; i < sim_state->num_vehicles; i++) {
    ThreadArgs* thread_args = malloc(sizeof(ThreadArgs));
    thread_args->sim_state = sim_state;
    thread_args->vehicle = &sim_state->vehicles[i];
    pthread_create(&threads[i], NULL, vehicle_thread, thread_args);
  }

  // Wait for all threads to complete.
  for (int i = 0; i < sim_state->num_vehicles; i++) {
    pthread_join(threads[i], NULL);
  }

  // Clean up simulation state.
  cleanup_simulation(sim_state);
  return 0;
}
