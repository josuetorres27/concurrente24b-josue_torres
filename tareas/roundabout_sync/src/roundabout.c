#include "roundabout.h"

int direction_to_index(char direction) {
  switch (direction) {
    case 'N': return 0;
    case 'E': return 1;
    case 'S': return 2;
    case 'O': return 3;
  }
  return -1;
}

char index_to_direction(int index) {
  switch (index) {
    case 0: return 'N';
    case 1: return 'E';
    case 2: return 'S';
    case 3: return 'O';
  }
  return '?';
}

long time_since_start(struct timespec start) {
  struct timespec now;
  clock_gettime(CLOCK_REALTIME, &now);
  return (now.tv_sec - start.tv_sec) * 1e3 +
    (now.tv_nsec - start.tv_nsec) / 1e6;
}

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
    fprintf(stderr, "Error: Vehicle %d has invalid entry or exit address: "
      "%c -> %c\n", vehicle_id, v->entry, v->exit);
    pthread_mutex_unlock(&sim_state->print_mutex);
    return NULL;
  }

  pthread_mutex_lock(&sim_state->print_mutex);
  printf("Thread %d started with entry %c and exit %c\n", vehicle_id, v->entry,
    v->exit);
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
    printf("Thread %d entered segment %c\n", vehicle_id,
      full_path[current_index]);
    pthread_mutex_unlock(&sim_state->print_mutex);

    trajectory->path[trajectory->path_index++] = full_path[current_index];

    if (sim_state->verbose_mode) {
      pthread_mutex_lock(&sim_state->print_mutex);
      printf("Verbose: Thread %d in segment %c at %ld ms\n",
        vehicle_id, full_path[current_index], time_since_start(start_time));
      pthread_mutex_unlock(&sim_state->print_mutex);
    }

    if (sim_state->max_time > 0) {
      int sleep_time = sim_state->min_time +
        rand() % (sim_state->max_time - sim_state->min_time + 1);
      usleep(sleep_time * 1000);
    }

    sem_post(&sim_state->segments[segment_index].capacity);

    pthread_mutex_lock(&sim_state->print_mutex);
    printf("Thread %d exited segment %c\n", vehicle_id,
      full_path[current_index]);
    pthread_mutex_unlock(&sim_state->print_mutex);

    current_index = (current_index + 1) % cycle_size;

    if (current_index == start_index) {
      has_completed_cycle = 1;
    }

  } while (!has_completed_cycle && full_path[current_index] != v->exit);

  trajectory->path[trajectory->path_index++] = v->exit;
  trajectory->path[trajectory->path_index] = '\0';

  pthread_mutex_lock(&sim_state->print_mutex);
  printf("Vehicle %d trajectory: %s\n", trajectory->vehicle_id,
    trajectory->path);
  printf("Thread %d finished\n", vehicle_id);
  pthread_mutex_unlock(&sim_state->print_mutex);

  free(thread_args);
  return NULL;
}

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

void cleanup_simulation(SimulationState* sim_state) {
  for (int i = 0; i < NUM_SEGMENTS; i++) {
    sem_destroy(&sim_state->segments[i].capacity);
  }
  pthread_mutex_destroy(&sim_state->print_mutex);
  free(sim_state);
}
