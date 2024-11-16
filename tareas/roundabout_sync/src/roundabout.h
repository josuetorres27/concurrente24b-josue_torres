#define _DEFAULT_SOURCE
#define _POSIX_C_SOURCE 199309L

#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <semaphore.h>
#include <unistd.h>
#include <time.h>
#include <string.h>

#define NUM_SEGMENTS 4
#define MAX_VEHICLES 100

typedef struct {
  int segment_capacity;
  sem_t capacity;
} Segment;

typedef struct {
  int id;
  char entry;
  char exit;
} Vehicle;

typedef struct {
  int vehicle_id;
  char path[100];
  int path_index;
} Trajectory;

typedef struct {
  Segment segments[NUM_SEGMENTS];
  Vehicle vehicles[MAX_VEHICLES];
  Trajectory trajectories[MAX_VEHICLES];
  int num_vehicles;
  pthread_mutex_t print_mutex;
  int min_time;
  int max_time;
  int verbose_mode;
} SimulationState;

typedef struct {
  SimulationState* sim_state;
  Vehicle* vehicle;
} ThreadArgs;

int direction_to_index(char direction);
char index_to_direction(int index);
long time_since_start(struct timespec start);
void* vehicle_thread(void* arg);
SimulationState* init_simulation(int min_time, int max_time, int verbose_mode,
  int segment_capacity);
void cleanup_simulation(SimulationState* sim_state);
