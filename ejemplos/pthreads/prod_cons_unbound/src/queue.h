// Copyright 2021 Jeisson Hidalgo-Cespedes <jeisson.hidalgo@ucr.ac.cr> CC-BY-4
// TODO(all): Implement a thread-safe queue

#ifndef QUEUE_H
#define QUEUE_H

#include <pthread.h>
#include <stdbool.h>

/**
 * @brief Represents a node in the queue.
 * 
 * @details Each node contains the data and a pointer to the next node in the
 * queue.
 */
typedef struct queue_node {
  size_t data;
  struct queue_node* next;
} queue_node_t;

/**
 * @brief Represents the queue data structure.
 * 
 * @details The queue is implemented using a linked list of queue_node_t
 * elements. It is protected by a mutex to ensure thread-safe access.
 */
typedef struct {
  pthread_mutex_t can_access_queue;  ///< Mutex for synchronizing access.
  queue_node_t* head;  ///< Pointer to the front of the queue (oldest element).
  queue_node_t* tail;  ///< Pointer to the back of the queue (newest element).
} queue_t;

/**
 * @todo: document all interfaces
 * @remarks This subroutine is NOT thread-safe
 */
int queue_init(queue_t* queue);

int queue_destroy(queue_t* queue);

/**
 * @remarks This subroutine is thread-safe
 */
bool queue_is_empty(queue_t* queue);
int queue_enqueue(queue_t* queue, const size_t data);
int queue_dequeue(queue_t* queue, size_t* data);
void queue_clear(queue_t* queue);

#endif  // QUEUE_H
