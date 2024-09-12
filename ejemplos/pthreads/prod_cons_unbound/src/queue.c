// Copyright 2021 Jeisson Hidalgo-Cespedes <jeisson.hidalgo@ucr.ac.cr> CC-BY-4

/**
 * @file queue.c
 * @brief Implements a thread-safe queue for the producer-consumer simulation.
 * 
 * @details This file provides the implementation for a queue data structure
 * that supports concurrent access by multiple threads. The queue is protected
 * by a mutex, allowing multiple producers and consumers to safely enqueue and
 * dequeue elements without race conditions.
 */

#include <assert.h>
#include <stdlib.h>

#include "queue.h"

/** Removes the first element from the queue without locking. */
void queue_remove_first_unsafe(queue_t* queue);
/** Checks if the queue is empty without locking. */
bool queue_is_empty_unsafe(queue_t* queue);

/**
 * @brief Initializes the queue.
 * 
 * @details This function sets up the internal state of the queue and
 * initializes the mutex used for thread safety. It must be called before the
 * queue can be used by any thread.
 * 
 * @param queue Pointer to the queue to initialize.
 * @return 0 on success, or a non-zero error code on failure.
 */
int queue_init(queue_t* queue) {
  assert(queue);
  int error = pthread_mutex_init(&queue->can_access_queue, /*attr*/ NULL);
  queue->head = NULL;
  queue->tail = NULL;
  return error;
}

/** Destroys the queue. */
int queue_destroy(queue_t* queue) {
  queue_clear(queue);
  return pthread_mutex_destroy(&queue->can_access_queue);
}

/**
 * @brief Checks if the queue is empty.
 * 
 * @details This function locks the queue's mutex to safely check whether
 * the queue contains any elements.
 * 
 * @param queue Pointer to the queue to check.
 * @return true if the queue is empty, false otherwise.
 */
bool queue_is_empty(queue_t* queue) {
  assert(queue);
  pthread_mutex_lock(&queue->can_access_queue);
  bool result = queue->head == NULL;
  pthread_mutex_unlock(&queue->can_access_queue);
  return result;
}

bool queue_is_empty_unsafe(queue_t* queue) {
  assert(queue);
  return queue->head == NULL;
}

/** creates a new node with the given data and appends it to the queue. */
int queue_enqueue(queue_t* queue, const size_t data) {
  assert(queue);
  int error = EXIT_SUCCESS;

  queue_node_t* new_node = (queue_node_t*) calloc(1, sizeof(queue_node_t));
  if (new_node) {
    new_node->data = data;

    pthread_mutex_lock(&queue->can_access_queue);
    if (queue->tail) {
      queue->tail = queue->tail->next = new_node;
    } else {
      queue->head = queue->tail = new_node;
    }
    pthread_mutex_unlock(&queue->can_access_queue);
  } else {
    error = EXIT_FAILURE;
  }

  return error;
}

/** Removes the node at the front of the queue and retrieves its data. */
int queue_dequeue(queue_t* queue, size_t* data) {
  assert(queue);
  int error = 0;

  pthread_mutex_lock(&queue->can_access_queue);
  if (!queue_is_empty_unsafe(queue)) {
    if (data) {
      *data = queue->head->data;
    }
    queue_remove_first_unsafe(queue);
  } else {
    error = EXIT_FAILURE;
  }
  pthread_mutex_unlock(&queue->can_access_queue);

  return error;
}

void queue_remove_first_unsafe(queue_t* queue) {
  assert(queue);
  assert(!queue_is_empty_unsafe(queue));
  queue_node_t* node = queue->head;
  queue->head = queue->head->next;
  free(node);
  if (queue->head == NULL) {
    queue->tail = NULL;
  }
}

/** Clears all elements from the queue. */
void queue_clear(queue_t* queue) {
  assert(queue);
  pthread_mutex_lock(&queue->can_access_queue);
  while (!queue_is_empty_unsafe(queue)) {
    queue_remove_first_unsafe(queue);
  }
  pthread_mutex_unlock(&queue->can_access_queue);
}
