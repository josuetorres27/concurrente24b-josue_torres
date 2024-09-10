// Copyright 2024 Esteban Solís <esteban.solisalfaro@ucr.ac.cr>
// Copyright 2024 Josué Torres <josue.torressibaja@ucr.ac.cr>

#define _XOPEN_SOURCE 500  // To use lrand48.
#ifndef TASK_HANDLER_H
#define TASK_HANDLER_H

#include <assert.h>
#include <inttypes.h>
#include <pthread.h>
#include <semaphore.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <unistd.h>

/**
 * @brief Structure for shared data between threads.
 *
 * @details Contains semaphores that control the order of building tasks.
 */
typedef struct shared_data {
  sem_t walls_ready;
  sem_t roof_ready;
  sem_t electrical_installation_ready;
  sem_t exterior_plumbing_ready;
  sem_t interior_plumbing_ready;
  sem_t interior_painting_ready;
  sem_t exterior_painting_ready;
  sem_t floor_ready;
} shared_data_t;

/**
 * @brief Structure for private data used by each thread.
 */
typedef struct private_data {
  shared_data_t* shared_data;
} private_data_t;

/**
 * @brief Executes the walls construction task.
 *
 * @details Simulates the walls building process. 
 * Signals the next dependent tasks by posting to the walls_ready semaphore.
 *
 * @param data Pointer to private data.
 * @return Always returns NULL.
 */
void* do_walls(void* data);

/**
 * @brief Executes the roof construction task.
 *
 * @details Waits for the walls to be completed before starting.
 * Signals the roof_ready semaphore when done.
 *
 * @see do_walls
 */
void* do_roof(void* data);

/**
 * @brief Executes the electrical installation task.
 * @see do_roof.
 */
void* do_electrical_installation(void* data);

/**
 * @brief Executes the exterior plumbing construction task.
 * @see do_roof.
 */
void* do_exterior_plumbing(void* data);

/**
 * @brief Executes the interior plumbing construction task.
 * @see do_roof.
 */
void* do_interior_plumbing(void* data);

/**
 * @brief Executes the interior painting task.
 * @see do_roof.
 */
void* do_interior_painting(void* data);

/**
 * @brief Executes the exterior painting task.
 * @see do_roof.
 */
void* do_exterior_painting(void* data);

/**
 * @brief Executes the floor construction task.
 * @see do_roof.
 */
void* do_floor(void* data);

/**
 * @brief Executes the interior finishes construction task.
 * @see do_roof.
 */
void* do_interior_finishes(void* data);

/**
 * @brief Executes the exterior finishes construction task.
 * @see do_roof.
 */
void* do_exterior_finishes(void* data);

#endif
