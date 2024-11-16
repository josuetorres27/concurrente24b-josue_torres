// Copyright 2024 Josue Torres Sibaja <josue.torressibaja@ucr.ac.cr>

/**
 * @file roundabout.c
 * @brief Implements the simulation logic for the roundabout traffic system.
 *
 * @details This file contains utility functions, threading logic, and the main
 * simulation setup for vehicles in the roundabout.
 */

#include "roundabout.h"

/**
 * @brief Converts a direction character to its corresponding index.
 *
 * @details The mapping is as follows:
 * - 'N' -> 0
 * - 'E' -> 1
 * - 'S' -> 2
 * - 'O' -> 3
 *
 * @param direction A character representing a direction ('N', 'E', 'S', 'O').
 * @return int The index of the direction, or -1 if the input is invalid.
 */
int direction_to_index(char direction) {
  switch (direction) {
    case 'N': return 0;
    case 'E': return 1;
    case 'S': return 2;
    case 'O': return 3;
  }
  return -1;
}

/**
 * @brief Converts a direction index to its corresponding character.
 *
 * @details The mapping is as follows:
 * - 0 -> 'N'
 * - 1 -> 'E'
 * - 2 -> 'S'
 * - 3 -> 'O'
 *
 * @param index An integer representing a direction index (0-3).
 * @return char Character for the direction, or '?' if the index is invalid.
 */
char index_to_direction(int index) {
  switch (index) {
    case 0: return 'N';
    case 1: return 'E';
    case 2: return 'S';
    case 3: return 'O';
  }
  return '?';
}

/**
 * @brief Calculates the elapsed time in nanoseconds since a given start time.
 *
 * @param start_time A 'timespec' structure representing the start time.
 * @return long The elapsed time in nanoseconds.
 */
long long time_since_start(struct timespec start_time) {
  struct timespec current_time;
  clock_gettime(CLOCK_REALTIME, &current_time);
  return (current_time.tv_sec - start_time.tv_sec) * 1000000000LL +
    (current_time.tv_nsec - start_time.tv_nsec);
}

/**
 * @brief Thread function for simulating a vehicle's movement through the
 * roundabout.
 *
 * @details Each thread represents a vehicle, which follows its trajectory
 * through the roundabout, entering and exiting segments while respecting
 * capacity constraints.
 *
 * @param arg A pointer to a 'ThreadArgs' structure containing simulation state
 * and vehicle information.
 * @return void* Always returns 'NULL'.
 */
void* vehicle_thread(void* arg) {
  ThreadArgs* thread_args = (ThreadArgs*) arg;
  SimulationState* sim_state = thread_args->sim_state;
  Vehicle* v = thread_args->vehicle;
  int vehicle_id = v->id;

  Trajectory* trajectory = &sim_state->trajectories[vehicle_id];
  trajectory->vehicle_id = vehicle_id;
  trajectory->path_index = 0;

  if (direction_to_index(v->entry) == -1 ||
    direction_to_index(v->exit) == -1) {
    pthread_mutex_lock(&sim_state->print_mutex);
    fprintf(stderr, "Error: Vehicle %d has invalid entry or exit address: %c "
      "-> %c\n", vehicle_id + 1, v->entry, v->exit);
    pthread_mutex_unlock(&sim_state->print_mutex);
    return NULL;
  }

  pthread_mutex_lock(&sim_state->print_mutex);
  //printf("Thread %d started with entry %c and exit %c\n", vehicle_id, v->entry,
  //  v->exit);
  pthread_mutex_unlock(&sim_state->print_mutex);

  struct timespec start_time;
  clock_gettime(CLOCK_REALTIME, &start_time);

  char full_path[] = {'N', 'O', 'S', 'E'};
  int cycle_size = 4;
  int start_index = 0;
  int has_completed_cycle = 0;

  while (full_path[start_index] != v->entry) {
    start_index = (start_index + 1) % cycle_size;
  }
  int current_index = start_index;

  do {
    int segment_index = direction_to_index(full_path[current_index]);

    sem_wait(&sim_state->segments[segment_index].capacity);

    pthread_mutex_lock(&sim_state->print_mutex);
    //printf("Thread %d entered segment %c\n", vehicle_id,
    //  full_path[current_index]);
    pthread_mutex_unlock(&sim_state->print_mutex);

    trajectory->path[trajectory->path_index++] = full_path[current_index];

    if (sim_state->verbose_mode) {
      pthread_mutex_lock(&sim_state->print_mutex);
      printf("%d: %c (Time since created: %lld ns)\n", vehicle_id + 1,
        full_path[current_index], time_since_start(start_time));
      pthread_mutex_unlock(&sim_state->print_mutex);
    }

    if (sim_state->max_time > 0) {
      int sleep_time = sim_state->min_time +
        rand() % (sim_state->max_time - sim_state->min_time + 1);
      usleep(sleep_time * 1000);
    }

    sem_post(&sim_state->segments[segment_index].capacity);

    pthread_mutex_lock(&sim_state->print_mutex);
    //printf("Thread %d exited segment %c\n", vehicle_id,
    //  full_path[current_index]);
    pthread_mutex_unlock(&sim_state->print_mutex);

    current_index = (current_index + 1) % cycle_size;

    if (current_index == start_index) {
      has_completed_cycle = 1;
    }
  } while (!has_completed_cycle && full_path[current_index] != v->exit);

  trajectory->path[trajectory->path_index++] = v->exit;
  trajectory->path[trajectory->path_index] = '\0';

  pthread_mutex_lock(&sim_state->print_mutex);
  // Print the formatted output.
  printf("%d %c%c: ", trajectory->vehicle_id + 1, v->entry, v->exit);
  for (int i = 0; i < trajectory->path_index; i++) {
    printf("%c", trajectory->path[i]);
    if (i < trajectory->path_index - 1) {
      printf(" ");
    }
  }
  printf("\n");
  //printf("Thread %d finished\n", vehicle_id);
  pthread_mutex_unlock(&sim_state->print_mutex);

  free(thread_args);
  return NULL;
}

/**
 * @brief Initializes the simulation state.
 *
 * @details Allocates memory for the simulation state, sets up segment
 * capacities, and initializes necessary synchronization mechanisms.
 *
 * @param min_time Minimum simulation time per segment.
 * @param max_time Maximum simulation time per segment.
 * @param verbose_mode Enables detailed logging if set to non-zero.
 * @param segment_capacity The capacity of each segment in the roundabout.
 * @return SimulationState* A pointer to the initialized simulation state.
 */
SimulationState* init_simulation(int min_time, int max_time, int verbose_mode,
  int segment_capacity) {
  SimulationState* sim_state = malloc(sizeof(SimulationState));

  sim_state->num_vehicles = 0;
  sim_state->min_time = min_time;
  sim_state->max_time = max_time;
  sim_state->verbose_mode = verbose_mode;

  pthread_mutex_init(&sim_state->print_mutex, NULL);

  for (int i = 0; i < NUM_SEGMENTS; i++) {
    sim_state->segments[i].segment_capacity = segment_capacity;
    sem_init(&sim_state->segments[i].capacity, 0, segment_capacity);
  }

  return sim_state;
}

/**
 * @brief Cleans up the simulation state.
 *
 * @details Destroys all synchronization mechanisms and frees allocated memory
 * for the simulation state.
 *
 * @param sim_state A pointer to the simulation state to be cleaned up.
 */
void cleanup_simulation(SimulationState* sim_state) {
  for (int i = 0; i < NUM_SEGMENTS; i++) {
    sem_destroy(&sim_state->segments[i].capacity);
  }
  pthread_mutex_destroy(&sim_state->print_mutex);
  free(sim_state);
}
