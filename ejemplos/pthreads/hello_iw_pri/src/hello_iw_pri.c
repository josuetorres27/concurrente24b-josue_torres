// Copyright 2021 Jeisson Hidalgo <jeisson.hidalgo@ucr.ac.cr> CC-BY 4.0

/**
 * @file hello_iw_pri.c
 * @brief Crea n cantidad de hilos secundarios, los cuales imprimen saludos.
 * 
 * @details El programa crea una cantidad arbitraria de hilos de ejecución.
 * Cada thread imprime "Hello from secondary thread i of w", donde i es el 
 * número de thread y w la cantidad total de hilos creada.
 */

#include <assert.h>
#include <inttypes.h>
#include <pthread.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

/**
 * @struct private_data
 * @brief Estructura para almacenar la información de un hilo.
 * 
 * Esta estructura almacena la información de un hilo, como su número 
 * identificador (rank), la cuenta total de hilos y un puntero al siguiente.
 */
// thread_private_data_t
typedef struct private_data {
  uint64_t thread_number; /**< Número del hilo (rank). */
  uint64_t thread_count; /**< Cuenta total de hilos. */
  struct private_data* next; /**< Puntero a la siguiente estructura private_data en una lista enlazada. */
} private_data_t;

void* greet(void* data);
int create_threads(uint64_t thread_count);

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

  error = create_threads(thread_count);
  return error;
}  // end procedure

/**
 * @brief Crea una cantidad especificada de hilos.
 * 
 * @param[in] thread_count Número de hilos secundarios a crear.
 * @return Código de estado.
 */
int create_threads(uint64_t thread_count) {
  int error = EXIT_SUCCESS;
  // for thread_number := 0 to thread_count do
  /**
   * @brief Reserva memoria para los identificadores de los hilos.
   */
  pthread_t* threads = (pthread_t*) malloc(thread_count * sizeof(pthread_t));
  /**
   * @brief Reserva memoria para la estructura de datos privada de cada hilo.
   */
  private_data_t* private_data = (private_data_t*)
    calloc(thread_count, sizeof(private_data_t));
  if (threads && private_data) { /** Verifica si la asignación de memoria fue exitosa. */
    for (uint64_t thread_number = 0; thread_number < thread_count
        ; ++thread_number) { /** Inicializa los datos privados del hilo actual. */
      private_data[thread_number].thread_number = thread_number;
      private_data[thread_number].thread_count = thread_count;
      // create_thread(greet, thread_number)
      error = pthread_create(&threads[thread_number], /*attr*/ NULL, greet
        , /*arg*/ &private_data[thread_number]);
      if (error == EXIT_SUCCESS) {
      } else {
        fprintf(stderr, "Error: could not create secondary thread\n");
        error = 21;
        break;
      }
    }

    // print "Hello from main thread"
    printf("Hello from main thread\n");

    for (uint64_t thread_number = 0; thread_number < thread_count
        ; ++thread_number) {
      pthread_join(threads[thread_number], /*value_ptr*/ NULL);
    }

    free(private_data);
    free(threads);
  } else {
    fprintf(stderr, "Error: could not allocate %" PRIu64 " threads\n"
      , thread_count);
    error = 22;
  }

  return error;
}

/**
 * @brief Imprime el saludo de cada hilo secundario y su rank.
 * 
 * @details Esta función utiliza los datos privados de cada hilo para 
 * imprimir un saludo con el número del hilo (rank) y el total de hilos.
 * @param[in] data Puntero a un valor que representa el rank del hilo.
 * @return Siempre retorna NULL.
 */
// procedure greet:
void* greet(void* data) {
  // assert(data);
  private_data_t* private_data = (private_data_t*) data; /** Convierte el puntero void* a private_data_t*. */
  // print "Hello from secondary thread"
  printf("Hello from secondary thread %" PRIu64 " of %" PRIu64 "\n"
    , (*private_data).thread_number, private_data->thread_count);
  return NULL;
}  // end procedure