// Copyright 2021 Jeisson Hidalgo-Cespedes <jeisson.hidalgo@ucr.ac.cr> CC-BY-4
// Simulates a producer and a consumer that share a bounded buffer

/**
 * @file prod_cons_bound.c
 * @brief Simulation of a producer-consumer problem with a bounded buffer.
 * 
 * @details This program creates a producer and a consumer that share a buffer
 * of limited capacity. Both of them operate concurrently using threads.
 * The producer fills the buffer with data, and the consumer reads the data.
 * Delays can be configured for both the producer and consumer to simulate
 * real-world processing time. The program takes command-line arguments that
 * define the buffer capacity, the number of rounds to produce and consume
 * data, and the minimum and maximum delays for the producer and consumer.
 */

/**
 * @def _DEFAULT_SOURCE
 * @brief Feature test macro to ensure compatibility with library functions.
 */
// @see `man feature_test_macros`
#define _DEFAULT_SOURCE

#include <assert.h>
#include <errno.h>
#include <pthread.h>
#include <semaphore.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/random.h>
#include <unistd.h>

/**
 * @brief Error codes used throughout the program.
 */
enum {
  ERR_NOMEM_SHARED = EXIT_FAILURE + 1,
  ERR_NOMEM_BUFFER,
  ERR_NO_ARGS,
  ERR_BUFFER_CAPACITY,
  ERR_ROUND_COUNT,
  ERR_MIN_PROD_DELAY,
  ERR_MAX_PROD_DELAY,
  ERR_MIN_CONS_DELAY,
  ERR_MAX_CONS_DELAY,
  ERR_CREATE_THREAD,
};

/**
 * @struct shared_data_t
 * @brief Structure for the shared data between producer and consumer.
 */
typedef struct {
  size_t thread_count; /**< Number of threads in the simulation. */
  size_t buffer_capacity; /**< Capacity of the shared buffer. */
  double* buffer; /**< Pointer to the shared buffer. */
  size_t rounds; /**< Number of rounds the producer and consumer will run. */
  useconds_t producer_min_delay; /**< Minimum delay for the producer. */
  useconds_t producer_max_delay; /**< Maximum delay for the producer. */
  useconds_t consumer_min_delay; /**< Minimum delay for the consumer. */
  useconds_t consumer_max_delay; /**< Maximum delay for the consumer. */
} shared_data_t;

/**
 * @brief Struct for the private data for each thread.
 */
typedef struct {
  size_t thread_number;
  shared_data_t* shared_data;
} private_data_t;

int analyze_arguments(int argc, char* argv[], shared_data_t* shared_data);
int create_threads(shared_data_t* shared_data);
void* produce(void* data);
void* consume(void* data);
useconds_t random_between(useconds_t min, useconds_t max);

/**
 * @brief Performs the simulation.
 * 
 * @details It makes different tasks to perform the simulation, like allocating
 * memory for the shared data structure; parsing and validating command-line
 * arguments to configure the simulation parameters; allocating memory for the
 * buffer; generating the random number; recording the start time of the
 * simulation; creating the producer and consumer threads; recording the end
 * time of the simulation and calculating the elapsed time; free the allocated
 * memory; and return an appropriate error code.
 * 
 * @return `EXIT_SUCCESS` if the program runs successfully, or an error code.
 */
int main(int argc, char* argv[]) {
  int error = EXIT_SUCCESS;
  shared_data_t* shared_data = (shared_data_t*)
    calloc(1, sizeof(shared_data_t));

  if (shared_data) {
    error = analyze_arguments(argc, argv, shared_data);
    if (error == EXIT_SUCCESS) {
      shared_data->buffer = (double*)
        calloc(shared_data->buffer_capacity, sizeof(double));
      if (shared_data->buffer) {
        unsigned int seed = 0u;
        getrandom(&seed, sizeof(seed), GRND_NONBLOCK);
        srandom(seed);

        struct timespec start_time;
        clock_gettime(/*clk_id*/CLOCK_MONOTONIC, &start_time);

        error = create_threads(shared_data);

        struct timespec finish_time;
        clock_gettime(/*clk_id*/CLOCK_MONOTONIC, &finish_time);

        double elapsed = (finish_time.tv_sec - start_time.tv_sec) +
          (finish_time.tv_nsec - start_time.tv_nsec) * 1e-9;
        printf("execution time: %.9lfs\n", elapsed);

        free(shared_data->buffer);
      } else {
        fprintf(stderr, "error: could not create buffer\n");
        error = ERR_NOMEM_BUFFER;
      }
    }

    free(shared_data);
  } else {
    fprintf(stderr, "Error: could not allocate shared data\n");
    error = ERR_NOMEM_SHARED;
  }

  return error;
}

/**
 * @brief Parses the command-line arguments and sets the values in shared_data.
 * 
 * @param argc Number of command-line arguments.
 * @param argv Array of command-line arguments.
 * @param shared_data Pointer to the shared data structure to populate.
 * @return EXIT_SUCCESS if arguments are valid, error code otherwise.
 */
int analyze_arguments(int argc, char* argv[], shared_data_t* shared_data) {
  int error = EXIT_SUCCESS;
  if (argc == 7) {
    if (sscanf(argv[1], "%zu", &shared_data->buffer_capacity) != 1
      || shared_data->buffer_capacity == 0) {
        fprintf(stderr, "error: invalid buffer capacity\n");
        error = ERR_BUFFER_CAPACITY;
    } else if (sscanf(argv[2], "%zu", &shared_data->rounds) != 1
      || shared_data->rounds == 0) {
        fprintf(stderr, "error: invalid round count\n");
        error = ERR_ROUND_COUNT;
    } else if (sscanf(argv[3], "%u", &shared_data->producer_min_delay) != 1) {
        fprintf(stderr, "error: invalid min producer delay\n");
        error = ERR_MIN_PROD_DELAY;
    } else if (sscanf(argv[4], "%u", &shared_data->producer_max_delay) != 1) {
        fprintf(stderr, "error: invalid max producer delay\n");
        error = ERR_MAX_PROD_DELAY;
    } else if (sscanf(argv[5], "%u", &shared_data->consumer_min_delay) != 1) {
        fprintf(stderr, "error: invalid min consumer delay\n");
        error = ERR_MIN_CONS_DELAY;
    } else if (sscanf(argv[6], "%u", &shared_data->consumer_max_delay) != 1) {
        fprintf(stderr, "error: invalid max consumer delay\n");
        error = ERR_MAX_CONS_DELAY;
    }
  } else {
    fprintf(stderr, "usage: prod_cons_bound buffer_capacity rounds"
      " producer_min_delay producer_max_delay"
      " consumer_min_delay consumer_max_delay\n");
      error = ERR_NO_ARGS;
  }
  return error;
}

/**
 * @brief Creates and starts the producer and consumer threads.
 * 
 * @param shared_data Pointer to the shared data structure.
 * @return EXIT_SUCCESS if threads are successfully created, error code
 * otherwise.
 */
int create_threads(shared_data_t* shared_data) {
  assert(shared_data);
  int error = EXIT_SUCCESS;

  pthread_t producer, consumer;
  error = pthread_create(&producer, /*attr*/ NULL, produce, shared_data);
  if (error == EXIT_SUCCESS) {
    error = pthread_create(&consumer, /*attr*/ NULL, consume, shared_data);
    if (error != EXIT_SUCCESS) {
      fprintf(stderr, "error: could not create consumer\n");
      error = ERR_CREATE_THREAD;
    }
  } else {
    fprintf(stderr, "error: could not create producer\n");
    error = ERR_CREATE_THREAD;
  }

  if (error == EXIT_SUCCESS) {
    pthread_join(producer, /*value_ptr*/ NULL);
    pthread_join(consumer, /*value_ptr*/ NULL);
  }

  return error;
}

/**
 * @brief Producer thread function.
 * 
 * @details The producer simulates generating data and storing it in a shared
 * bounded buffer. It produces 'rounds' sets of data, where each set consists
 * of filling the buffer to its capacity. For each item in the buffer, the
 * producer simulates a delay (randomly chosen between a minimum and maximum
 * delay) to represent the time taken to produce the item. After producing an
 * item, it stores it in the buffer.
 * 
 * @param data Pointer to the shared data structure.
 * 
 * @return Always returns NULL.
 */
void* produce(void* data) {
  // const private_data_t* private_data = (private_data_t*)data;
  shared_data_t* shared_data = (shared_data_t*)data;
  size_t count = 0;
  for (size_t round = 0; round < shared_data->rounds; ++round) {
    for (size_t index = 0; index < shared_data->buffer_capacity; ++index) {
      /** Delay to simulate that the producer is busy. */
      usleep(1000 * random_between(shared_data->producer_min_delay
        , shared_data->producer_max_delay));
      /** Increments the count of data inside the buffer. */
      shared_data->buffer[index] = ++count;
      printf("Produced %lg\n", shared_data->buffer[index]);
    }
  }

  return NULL;
}

/**
 * @brief Consumer thread function.
 * 
 * @details The consumer simulates consuming data from the buffer. It runs for
 * `rounds` iterations, where each iteration consists of reading and consuming
 * all items in the buffer. For each item in the buffer, the consumer also
 * introduces a random delay to simulate the time it takes to consume the item.
 * After consuming an item, it prints the consumed value.
 * 
 * @param data Pointer to the shared data structure.
 * 
 * @return Always returns NULL.
 */
void* consume(void* data) {
  // const private_data_t* private_data = (private_data_t*)data;
  shared_data_t* shared_data = (shared_data_t*)data;
  for (size_t round = 0; round < shared_data->rounds; ++round) {
    for (size_t index = 0; index < shared_data->buffer_capacity; ++index) {
      double value = shared_data->buffer[index];
      /** Delay to simulate that the consumer is busy. */
      usleep(1000 * random_between(shared_data->consumer_min_delay
        , shared_data->consumer_max_delay));
      printf("\tConsumed %lg\n", value);
    }
  }

  return NULL;
}

/**
 * @brief Generates a random delay between min and max.
 * 
 * @param min Minimum delay in microseconds.
 * @param max Maximum delay in microseconds.
 * @return A random delay between min and max.
 */
useconds_t random_between(useconds_t min, useconds_t max) {
  return min + (max > min ? (random() % (max - min)) : 0);
}
