// Copyright 2021 Jeisson Hidalgo-Cespedes <jeisson.hidalgo@ucr.ac.cr> CC-BY-4

#include <stdbool.h>
#include <stdio.h>

#include "common.h"
#include "consumer.h"

void* consume(void* data) {
  /** Casts the data parameter to a pointer of type simulation_t. */
  simulation_t* simulation = (simulation_t*)data;

  while (true) {
    /** Locks the mutex. */
    pthread_mutex_lock(&simulation->can_access_consumed_count);
    if (simulation->consumed_count < simulation->unit_count) {
      /** Reserves the next product. */
      ++simulation->consumed_count;
    } else {
      /** Unlocks the mutex. */
      pthread_mutex_unlock(&simulation->can_access_consumed_count);
      // break while
      break;
    }
    // unlock(can_access_consumed_count)
    pthread_mutex_unlock(&simulation->can_access_consumed_count);

    /** Waits untill it can consume. */
    sem_wait(&simulation->can_consume);

    size_t value = 0;
    queue_dequeue(&simulation->queue, &value);
    printf("\tConsuming %zu\n", value);
    usleep(1000 * random_between(simulation->consumer_min_delay
      , simulation->consumer_max_delay));
  }

  return NULL;
}
