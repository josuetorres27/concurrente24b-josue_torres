// Copyright 2021 Jeisson Hidalgo <jeisson.hidalgo@ucr.ac.cr> CC-BY 4.0

/**
 * @file hello_order_semaphor.c
 * @brief Crea n cantidad de hilos secundarios, los cuales imprimen saludos.
 * 
 * @details El programa crea una cantidad arbitraria de hilos de ejecución.
 * Cada thread imprime "Hello from secondary thread i of w", donde i es el 
 * número de thread y w la cantidad total de hilos creada. Los hilos creados 
 * saludan siempre en orden. Es decir, si se crean w threads, la salida será 
 * siempre en el orden correcto (sumando 1 al número del hilo anterior). Se 
 * utilizan semáforos como mecanismo de sincronización (control de 
 * concurrencia).
 */

#include <assert.h>
#include <inttypes.h>
#include <pthread.h>
#include <semaphore.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <unistd.h>

// thread_shared_data_t
typedef struct shared_data {
  /** Arreglo de semáforos al que todos los hilos tienen acceso. */
  sem_t* can_greet;
  uint64_t thread_count;
} shared_data_t;

// thread_private_data_t
typedef struct private_data {
  uint64_t thread_number;  // rank
  shared_data_t* shared_data;
} private_data_t;

void* greet(void* data);
int create_threads(shared_data_t* shared_data);

// procedure main(argc, argv[])
int main(int argc, char* argv[]) {
  int error = EXIT_SUCCESS;
  // create thread_count as result of converting argv[1] to integer
  // thread_count := integer(argv[1])
  uint64_t thread_count = sysconf(_SC_NPROCESSORS_ONLN);
  if (argc == 2) {
    if (sscanf(argv[1], "%" SCNu64, &thread_count) == 1) {
    } else {
      fprintf(stderr, "Error: invalid thread count\n");
      return 11;
    }
  }

  shared_data_t* shared_data = (shared_data_t*)calloc(1, sizeof(shared_data_t));  // NOLINT
  if (shared_data) {
    /** Reserva memoria para los semáforos, uno por cada hilo. */
    shared_data->can_greet = (sem_t*) calloc(thread_count, sizeof(sem_t));  // NOLINT
    shared_data->thread_count = thread_count;

    for (uint64_t thread_number = 0; thread_number < shared_data->thread_count
        ; ++thread_number) {
      // can_greet[thread_number] := create_semaphore(not thread_number)
      /**
       * @brief Inicializa uno por uno los semáforos, recibiendo la negación de
       * thread_number como parámetro 'value', lo que representa el valor del
       * semáforo. Se inicializa como 1 para el primer hilo y como 0 para los
       * demás.
       */
      error = sem_init(&shared_data->can_greet[thread_number], /*pshared*/ 0
        , /*value*/ !thread_number);
    }

    if (shared_data->can_greet) {
      struct timespec start_time, finish_time;
      clock_gettime(CLOCK_MONOTONIC, &start_time);

      error = create_threads(shared_data);

      clock_gettime(CLOCK_MONOTONIC, &finish_time);
      double elapsed_time = finish_time.tv_sec - start_time.tv_sec +
        (finish_time.tv_nsec - start_time.tv_nsec) * 1e-9;

      printf("Execution time: %.9lfs\n", elapsed_time);

      free(shared_data->can_greet);
    } else {
      fprintf(stderr, "Error: could not allocate semaphores\n");
      error = 13;
    }
    free(shared_data);
  } else {
    fprintf(stderr, "Error: could not allocate shared data\n");
    error = 12;
  }
  return error;
}  // end procedure


int create_threads(shared_data_t* shared_data) {
  int error = EXIT_SUCCESS;
  // for thread_number := 0 to thread_count do
  pthread_t* threads = (pthread_t*)  // NOLINT
    malloc(shared_data->thread_count * sizeof(pthread_t));
  private_data_t* private_data = (private_data_t*)  // NOLINT
    calloc(shared_data->thread_count, sizeof(private_data_t));
  if (threads && private_data) {
    for (uint64_t thread_number = 0; thread_number < shared_data->thread_count
        ; ++thread_number) {
      if (error == EXIT_SUCCESS) {
        private_data[thread_number].thread_number = thread_number;
        private_data[thread_number].shared_data = shared_data;
        // create_thread(greet, thread_number)
        error = pthread_create(&threads[thread_number], /*attr*/ NULL, greet
          , /*arg*/ &private_data[thread_number]);
        if (error == EXIT_SUCCESS) {
        } else {
          fprintf(stderr, "Error: could not create secondary thread\n");
          error = 21;
          break;
        }

      } else {
        fprintf(stderr, "Error: could not init semaphore\n");
        error = 22;
        break;
      }
    }

    // print "Hello from main thread"
    printf("Hello from main thread\n");

    for (uint64_t thread_number = 0; thread_number < shared_data->thread_count
        ; ++thread_number) {
      pthread_join(threads[thread_number], /*value_ptr*/ NULL);
      /** Destruir la memoria que se reservó para los semáforos. */
      sem_destroy(&shared_data->can_greet[thread_number]);
    }

    free(private_data);
    free(threads);
  } else {
    fprintf(stderr, "Error: could not allocate %" PRIu64 " threads\n"
      , shared_data->thread_count);
    error = 23;
  }

  return error;
}

// procedure greet:
void* greet(void* data) {
  assert(data);
  private_data_t* private_data = (private_data_t*) data;  // NOLINT
  shared_data_t* shared_data = private_data->shared_data;

  // Wait until it is my turn
  // wait(can_greet[thread_number])
  /**
   * @brief Pone a esperar a cada semáforo.
   * @details Espera a que el valor del semáforo sea mayor que 0, y luego lo
   * decrementa. Si el valor del semáforo es 0, la función hace que el hilo se
   * bloquee (espere) hasta que otro hilo incremente el valor del semáforo.
  */
  int error = sem_wait(&shared_data->can_greet[private_data->thread_number]);
  if (error) {
    fprintf(stderr, "error: could not wait for semaphore\n");
  }

  // print "Hello from secondary thread"
  /**
   * @brief Cuando un hilo es desbloqueado por el semáforo, imprime su saludo.
   * Esto asegura que los hilos saluden uno por uno.
   */
  printf("Hello from secondary thread %" PRIu64 " of %" PRIu64 "\n"
    , private_data->thread_number, shared_data->thread_count);

  // Allow subsequent thread to do the task
  // signal(can_greet[(thread_number + 1) mod thread_count])
  const uint64_t next_thread = (private_data->thread_number + 1)
    % shared_data->thread_count;
  /**
   * @brief Se pone el valor del semáforo en +1.
   * @details Incrementa el valor del semáforo en 1, lo que desbloquea
   * cualquier hilo que esté esperando en él (si había algún hilo bloqueado por
   * sem_wait). Luego, se desbloquea el hilo que sigue, permitiéndole continuar
   * su ejecución.
   */
  error = sem_post(&shared_data->can_greet[next_thread]);
  if (error) {
    fprintf(stderr, "error: could not increment semaphore\n");
  }

  return NULL;
}  // end procedure
