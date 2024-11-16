#include "roundabout.h"

int main(int argc, char* argv[]) {
  int min_time = 0, max_time = 0, verbose_mode = 0;

  if (argc >= 3) {
    min_time = atoi(argv[1]);
    max_time = atoi(argv[2]);
  }
  if (argc == 4 && strcmp(argv[3], "-v") == 0) {
    verbose_mode = 1;
  }

  int segment_capacity;
  printf("Enter the capacity of each segment: ");
  if (scanf("%d", &segment_capacity) != 1 || segment_capacity <= 0) {
    fprintf(stderr, "Error: Capacity must be a positive integer.\n");
    return 1;
  }

  SimulationState* sim_state = init_simulation(min_time, max_time,
    verbose_mode, segment_capacity);

  printf("Enter vehicles (format: input output, e.g., NE):\n");
  char entry, exit;
  while (scanf(" %c %c", &entry, &exit) == 2 &&
    sim_state->num_vehicles < MAX_VEHICLES) {
    sim_state->vehicles[sim_state->num_vehicles].id = sim_state->num_vehicles;
    sim_state->vehicles[sim_state->num_vehicles].entry = entry;
    sim_state->vehicles[sim_state->num_vehicles].exit = exit;
    sim_state->num_vehicles++;
  }

  srand(time(NULL));

  pthread_t threads[MAX_VEHICLES];
  for (int i = 0; i < sim_state->num_vehicles; i++) {
    ThreadArgs* thread_args = malloc(sizeof(ThreadArgs));
    thread_args->sim_state = sim_state;
    thread_args->vehicle = &sim_state->vehicles[i];
    pthread_create(&threads[i], NULL, vehicle_thread, thread_args);
  }

  for (int i = 0; i < sim_state->num_vehicles; i++) {
    pthread_join(threads[i], NULL);
  }

  cleanup_simulation(sim_state);
  return 0;
}
