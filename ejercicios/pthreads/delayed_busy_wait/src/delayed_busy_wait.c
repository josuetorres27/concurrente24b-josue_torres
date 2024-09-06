// Copyright 2021 Jeisson Hidalgo <jeisson.hidalgo@ucr.ac.cr> CC-BY 4.0

/**
 * @file delayed_busy_wait.c
 * @brief Crea n cantidad de hilos secundarios, los cuales imprimen saludos.
 * 
 * @details El programa crea una cantidad de hilos de ejecución especificada 
 * por parámetro a través de la línea de comandos. Cada thread imprime "Hello 
 * from secondary thread i of w", donde i es el número de thread y w la 
 * cantidad total de hilos creada. Los hilos saludan siempre en orden. Es 
 * decir, si se crean w threads, la salida será siempre en el orden correcto 
 * (sumando 1 al número del hilo anterior). El programa utiliza un mecanismo 
 * de espera activa con retraso constante, especificado por el usuario en 
 * microsegundos como segundo argumento en la línea de comandos. Si no es el 
 * turno de un hilo, este espera una cantidad fija de microsegundos antes de 
 * volver a verificar si es su turno.
 */

#include <assert.h>
#include <inttypes.h>
#include <pthread.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <unistd.h>

// thread_shared_data_t
typedef struct shared_data {
  uint64_t next_thread;
  uint64_t thread_count;
  useconds_t delay; /** Almacena el tiempo de retraso en microsegundos. */
} shared_data_t;

// thread_private_data_t
typedef struct private_data {
  uint64_t thread_number;
  shared_data_t* shared_data;
} private_data_t;

void* greet(void* data);
int create_threads(shared_data_t* shared_data);

int main(int argc, char* argv[]) {
  int error = EXIT_SUCCESS;
  uint64_t thread_count = sysconf(_SC_NPROCESSORS_ONLN);
  useconds_t delay = 1000; /** Valor por defecto de espera en microsegundos. */

  /** Comprobar los argumentos de línea de comandos. */
  if (argc >= 2) {
    if (sscanf(argv[1], "%" SCNu64, &thread_count) != 1) {
      fprintf(stderr, "Error: invalid thread count\n");
      return 11;
    }
  }

  if (argc >= 3) {
    if (sscanf(argv[2], "%u", &delay) != 1) {
      fprintf(stderr, "Error: invalid delay value\n");
      return 12;
    }
  }

  shared_data_t* shared_data = (shared_data_t*) calloc(1,
    sizeof(shared_data_t));
  if (shared_data) {
    shared_data->next_thread = 0;
    shared_data->thread_count = thread_count;
    /**
     * @brief Guardar el valor de retraso en la estructura.
     */
    shared_data->delay = delay;

    struct timespec start_time, finish_time;
    clock_gettime(CLOCK_MONOTONIC, &start_time);

    error = create_threads(shared_data);

    clock_gettime(CLOCK_MONOTONIC, &finish_time);
    double elapsed_time = finish_time.tv_sec - start_time.tv_sec +
      (finish_time.tv_nsec - start_time.tv_nsec) * 1e-9;

    printf("Execution time: %.9lfs\n", elapsed_time);

    free(shared_data);
  } else {
    fprintf(stderr, "Error: could not allocate shared data\n");
    return 13;
  }
  return error;
}

int create_threads(shared_data_t* shared_data) {
  int error = EXIT_SUCCESS;
  pthread_t* threads = (pthread_t*) malloc(shared_data->thread_count *
    sizeof(pthread_t));
  private_data_t* private_data = (private_data_t*) calloc(shared_data->
    thread_count, sizeof(private_data_t));

  if (threads && private_data) {
    for (uint64_t thread_number = 0; thread_number < shared_data->thread_count;
      ++thread_number) {
      private_data[thread_number].thread_number = thread_number;
      private_data[thread_number].shared_data = shared_data;
      error = pthread_create(&threads[thread_number], NULL, greet,
        &private_data[thread_number]);
      if (error != EXIT_SUCCESS) {
        fprintf(stderr, "Error: could not create thread\n");
        error = 21;
        break;
      }
    }
    printf("Hello from main thread\n");
    for (uint64_t thread_number = 0; thread_number < shared_data->thread_count;
      ++thread_number) {
      pthread_join(threads[thread_number], NULL);
    }

    free(private_data);
    free(threads);
  } else {
    fprintf(stderr, "Error: could not allocate threads\n");
    error = 22;
  }
  return error;
}

void* greet(void* data) {
  assert(data);
  private_data_t* private_data = (private_data_t*) data;
  shared_data_t* shared_data = private_data->shared_data;
  /**
   * @brief Espera activa con retraso constante.
   */
  while (shared_data->next_thread < private_data->thread_number) {
    usleep(shared_data->delay);
  }
  /**
   * @brief Espera activa con retraso pseudoaleatorio.
   */
  /*
  while (next_thread < my_thread_id) {
    const unsigned my_delay = rand_r(&my_seed) % max_delay;
    usleep(my_delay);
  }
  */
  printf("Hello from secondary thread %" PRIu64 " of %" PRIu64 "\n",
    private_data->thread_number, shared_data->thread_count);
  ++shared_data->next_thread;
  return NULL;
}
