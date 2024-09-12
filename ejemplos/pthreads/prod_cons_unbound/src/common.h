// Copyright 2021 Jeisson Hidalgo-Cespedes <jeisson.hidalgo@ucr.ac.cr> CC-BY-4

#ifndef COMMON_H
#define COMMON_H

#include <semaphore.h>
#include <stdlib.h>
#include <unistd.h>

#include "queue.h"

enum {
  ERR_NOMEM_SHARED = EXIT_FAILURE + 1,
  ERR_NOMEM_BUFFER,
  ERR_NO_ARGS,
  ERR_UNIT_COUNT,
  ERR_PRODUCER_COUNT,
  ERR_CONSUMER_COUNT,
  ERR_MIN_PROD_DELAY,
  ERR_MAX_PROD_DELAY,
  ERR_MIN_CONS_DELAY,
  ERR_MAX_CONS_DELAY,
  ERR_CREATE_THREAD,
};

/**
 * @brief Holds the state of the producer-consumer simulation.
 * 
 * @details This structure manages all the data shared between producers and
 * consumers, including the unit counts, delays, and synchronization mechanisms
 * (such as semaphores and mutexes). The buffer where producers store data and
 * consumers retrieve it is also contained within this structure.
 */
typedef struct simulation {
  size_t unit_count;
  size_t producer_count;
  size_t consumer_count;
  useconds_t producer_min_delay;
  useconds_t producer_max_delay;
  useconds_t consumer_min_delay;
  useconds_t consumer_max_delay;

  queue_t queue;  ///< Shared buffer between producers and consumers.
  /** Mutex to synchronize access to the next unit of production. */
  pthread_mutex_t can_access_next_unit;
  size_t next_unit;  ///< Next unit of production to be produced.
  sem_t can_consume;  ///< Semaphore indicating the number of consumable units.
  /** Mutex to synchronize access to the consumed unit count. */
  pthread_mutex_t can_access_consumed_count;
  size_t consumed_count;  ///< Counter for the number of consumed units.
} simulation_t;

useconds_t random_between(useconds_t min, useconds_t max);

#endif  // COMMON_H
