// Copyright 2024 Josue Torres Sibaja <josue.torressibaja@ucr.ac.cr>

#define _DEFAULT_SOURCE  ///< To use 'usleep()'.
#define _POSIX_C_SOURCE 199309L  ///< To use 'CLOCK_REALTIME'.

#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <semaphore.h>
#include <unistd.h>
#include <time.h>
#include <string.h>

#define NUM_SEGMENTS 4  ///< Number of segments in the roundabout.
#define MAX_VEHICLES 2000  ///< Maximum number of vehicles in the roundabout.

/**
 * @struct Segment
 * @brief Represents a single segment of the roundabout.
 *
 * @details Each segment has a capacity limit and a semaphore to control
 * access.
 */
typedef struct {
  int segment_capacity;  ///< The maximum number of vehicles allowed.
  sem_t capacity;        ///< Semaphore to enforce capacity constraints.
} Segment;

/**
 * @struct Vehicle
 * @brief Represents a vehicle in the simulation.
 *
 * @details Each vehicle has a unique ID, an entry direction, and an exit
 * direction.
 */
typedef struct {
  int id;       ///< Unique identifier for the vehicle.
  char entry;   ///< Entry direction of the vehicle ('N', 'E', 'S', 'O').
  char exit;    ///< Exit direction of the vehicle ('N', 'E', 'S', 'O').
} Vehicle;

/**
 * @struct Trajectory
 * @brief Tracks the path taken by a vehicle through the roundabout.
 *
 * @details The trajectory includes the vehicle ID, the path as a sequence of
 * segments, and the current index in the path array.
 */
typedef struct {
  int vehicle_id;  ///< ID of the vehicle whose trajectory is being tracked.
  char path[100];  ///< Array storing the sequence of segments visited.
  int path_index;  ///< Current index in the path array.
} Trajectory;

/**
 * @struct SimulationState
 * @brief Represents the entire state of the roundabout simulation.
 *
 * @details Contains information about the segments, vehicles, trajectories,
 * and configuration settings.
 */
typedef struct {
  int min_time;  ///< Minimum time a vehicle spends in a segment.
  int max_time;  ///< Maximum time a vehicle spends in a segment.
  int verbose_mode;  ///< Flag to enable verbose logging (1 for enabled).
  int num_vehicles;  ///< Current number of vehicles in the simulation.
  int next_to_print;  ///< Indicates the next thread that can print.
  pthread_cond_t order_cond;  ///< Mutex for synchronizing print operations.
  pthread_mutex_t print_mutex;  ///< Mutex to prevent simultaneous printing.
  pthread_mutex_t order_mutex;  ///< Mutex to control the printing order.
  Segment segments[NUM_SEGMENTS];  ///< Array of segments in the roundabout.
  Vehicle vehicles[MAX_VEHICLES];  ///< Array of vehicles in the simulation.
  // Array of trajectories for each vehicle.
  Trajectory trajectories[MAX_VEHICLES];
} SimulationState;

/**
 * @struct ThreadArgs
 * @brief Holds arguments to be passed to vehicle threads.
 *
 * @details Combines simulation state and a reference to a specific vehicle.
 */
typedef struct {
  SimulationState* sim_state;  ///< Pointer to the shared simulation state.
  Vehicle* vehicle;  ///< Pointer to the vehicle associated with the thread.
} ThreadArgs;

// Declaration of roundabout simulation functions.
int direction_to_index(char direction);
char index_to_direction(int index);
long long time_since_start(struct timespec start_time);
void* vehicle_thread(void* arg);
SimulationState* init_simulation(int min_time, int max_time, int verbose_mode,
  int segment_capacity);
void cleanup_simulation(SimulationState* sim_state);
