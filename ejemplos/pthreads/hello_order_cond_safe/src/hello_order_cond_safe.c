// Copyright 2021 Jeisson Hidalgo <jeisson.hidalgo@ucr.ac.cr> CC-BY 4.0

/**
 * @file hello_order_cond_safe.c
 * @brief Crea n cantidad de hilos secundarios, los cuales imprimen saludos.
 * 
 * @details El programa crea una cantidad arbitraria de hilos de ejecución.
 * Cada thread imprime "Hello from secondary thread i of w", donde i es el 
 * número de thread y w la cantidad total de hilos creada. Los hilos creados 
 * saludan siempre en orden. Es decir, si se crean w threads, la salida será 
 * siempre en el orden correcto (sumando 1 al número del hilo anterior). Se 
 * utiliza seguridad condicional para evitar el control de concurrencia.
 */

#include <assert.h>
#include <inttypes.h>
#include <pthread.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <unistd.h>

/**
 * @brief Define una constante que se utilizará a lo largo del programa para
 * especificar el tamaño máximo de las cadenas de texto.
 */
#define MAX_GREET_LEN 256

// thread_shared_data_t
typedef struct shared_data {
  char** greets; /**< Almacena los saludos generados por cada hilo. */
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

  /**
   * @brief Se utiliza calloc para reservar memoria para los saludos de los
   * hilos. Después de la asignación se verifica si esta fue exitosa revisando
   * si el puntero resultante es diferente de NULL. Si la asignación falla, el
   * programa devuelve un error.
   */
  shared_data_t* shared_data = (shared_data_t*) calloc(1,  // NOLINT
    sizeof(shared_data_t));
  if (shared_data) {
    shared_data->greets = (char**) calloc(thread_count, sizeof(char*));  // NOLINT
    shared_data->thread_count = thread_count;

    if (shared_data->greets) {
      struct timespec start_time, finish_time;
      clock_gettime(CLOCK_MONOTONIC, &start_time);

      error = create_threads(shared_data);

      clock_gettime(CLOCK_MONOTONIC, &finish_time);
      double elapsed_time = finish_time.tv_sec - start_time.tv_sec +
        (finish_time.tv_nsec - start_time.tv_nsec) * 1e-9;

      printf("Execution time: %.9lfs\n", elapsed_time);

      free(shared_data->greets);
    } else {
      fprintf(stderr, "Error: could not allocate greets\n");
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
    malloc(shared_data->thread_count * sizeof(pthread_t));  // NOLINT
  private_data_t* private_data = (private_data_t*)  // NOLINT
    calloc(shared_data->thread_count, sizeof(private_data_t));
  if (threads && private_data) {
    for (uint64_t thread_number = 0; thread_number < shared_data->thread_count;
      ++thread_number) {
      /**
       * @brief Para cada hilo se asigna memoria suficiente para almacenar una
       * cadena de caracteres de longitud máxima, la cual se definió al incio.
       */
      shared_data->greets[thread_number] = (char*)  // NOLINT
        malloc(MAX_GREET_LEN * sizeof(char));
      if (shared_data->greets[thread_number]) {
        // *shared_data->greets[thread_number] = '\0';
        /** Si se logra reservar memoria, se pone un caracter vacío. */
        shared_data->greets[thread_number][0] = '\0';
        private_data[thread_number].thread_number = thread_number;
        private_data[thread_number].shared_data = shared_data;
        // create_thread(greet, thread_number)
        /**
         * @brief Al crear los hilos, el código revisa el resultado de
         * pthread_create. Si se produce un error al crear un hilo, el programa
         * muestra un mensaje de error y termina la creación de más hilos.
         */
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
      /** Esperar a que cada hilo termine. */
      pthread_join(threads[thread_number], /*value_ptr*/ NULL);
    }

    // for thread_number := 0 to thread_count do
    /** Imprime cada saludo almacenado en el arreglo. */
    for (uint64_t thread_number = 0; thread_number < shared_data->thread_count
        ; ++thread_number) {
      // print greets[thread_number]
      printf("%s\n", shared_data->greets[thread_number]);
      free(shared_data->greets[thread_number]);
    }  // end for

    free(private_data);
    free(threads);
  } else {
    fprintf(stderr, "Error: could not allocate %" PRIu64 " threads\n"
      , shared_data->thread_count);
    error = 22;
  }

  return error;
}

// procedure greet:
void* greet(void* data) {
  assert(data); /**< Verifica que data no sea NULL antes de proceder. */
  private_data_t* private_data = (private_data_t*) data;  // NOLINT
  shared_data_t* shared_data = private_data->shared_data;

  // greets[thread_number] := format("Hello from secondary thread"
  // , thread_number, " of ", thread_count)
  /**
   * @brief Cada hilo genera un saludo utilizando sprintf y lo almacena en su
   * respectivo índice en el arreglo greets.
   */
  snprintf(shared_data->greets[private_data->thread_number]
    , "Hello from secondary thread %" PRIu64 " of %" PRIu64
    , private_data->thread_number, shared_data->thread_count);

  return NULL;
}  // end procedure
