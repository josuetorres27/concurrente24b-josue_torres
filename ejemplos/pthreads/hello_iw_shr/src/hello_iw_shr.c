// Copyright 2021 Jeisson Hidalgo <jeisson.hidalgo@ucr.ac.cr> CC-BY 4.0

/**
 * @file hello_iw_shr.c
 * @brief Crea n cantidad de hilos secundarios, los cuales imprimen saludos.
 * 
 * @details El programa crea una cantidad arbitraria de hilos de ejecución.
 * Cada thread imprime "Hello from secondary thread i of w", donde i es el 
 * número de thread y w la cantidad total de hilos creada. Además, los datos 
 * comunes para todos los hilos de ejecución están en un registro (estructura) 
 * compartida para cada hilo, manteniendo los datos exclusivos para cada hilo 
 * en el registro privado, y se reporta la duración en segundos que los hilos 
 * toman en saludar.
 */

#include <assert.h>
#include <inttypes.h>
#include <pthread.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h> /**< Para reportar los segundos que los hilos toman en saludar. */
#include <unistd.h>

/**
 * @struct shared_data
 * @brief Estructura que almacena datos compartidos entre todos los hilos.
 * 
 * @details Esta estructura almacena información compartida por todos los 
 * hilos, como la cantidad total de hilos (thread_count).
 */
// thread_shared_data_t
typedef struct shared_data {
  uint64_t thread_count;
} shared_data_t;

/**
 * @struct private_data
 * @brief Estructura que almacena datos privados de cada hilo.
 * 
 * Esta estructura se utiliza para almacenar información específica de cada 
 * hilo, como su número (rank) y un puntero a la estructura shared_data.
 */
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
 * @brief Reserva memoria para la estructura de datos compartidos entre hilos.
 */
  shared_data_t* shared_data = (shared_data_t*)calloc(1, sizeof(shared_data_t));
  if (shared_data) { /**< Verifica si la reserva de memoria fue exitosa. */
    shared_data->thread_count = thread_count; /**< Asigna el número de hilos a los datos compartidos. */

    struct timespec start_time, finish_time; /**< Declara variables para medir el tiempo de ejecución. */
    clock_gettime(CLOCK_MONOTONIC, &start_time); /**< Registra el tiempo de inicio antes de crear los hilos. */

    error = create_threads(shared_data);
    /**
     * @brief Registra el tiempo de finalización después de que todos los hilos han terminado.
     */
    clock_gettime(CLOCK_MONOTONIC, &finish_time);
    double elapsed_time = finish_time.tv_sec - start_time.tv_sec +
      (finish_time.tv_nsec - start_time.tv_nsec) * 1e-9; /**< Calcula el tiempo de ejecución en segundos. */

    printf("Execution time: %.9lfs\n", elapsed_time);

    free(shared_data);
  } else {
    fprintf(stderr, "Error: could not allocate shared data\n");
    return 12;
  }
  return error;
}  // end procedure

int create_threads(shared_data_t* shared_data) {
  int error = EXIT_SUCCESS;
  // for thread_number := 0 to thread_count do
  pthread_t* threads = (pthread_t*)
    malloc(shared_data->thread_count * sizeof(pthread_t));
  /**
   * @brief Asigna memoria para los datos privados de cada hilo.
   */
  private_data_t* private_data = (private_data_t*)
    calloc(shared_data->thread_count, sizeof(private_data_t));
  if (threads && private_data) {
    /**
     * @brief Asigna el número del hilo (rank) a la estructura de datos privada.
     */
    for (uint64_t thread_number = 0; thread_number < shared_data->thread_count
        ; ++thread_number) {
      /**
       * @brief Asigna el puntero a los datos compartidos en la estructura de datos privada.
       */
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
    }

    // print "Hello from main thread"
    printf("Hello from main thread\n");

    for (uint64_t thread_number = 0; thread_number < shared_data->thread_count
        ; ++thread_number) {
      pthread_join(threads[thread_number], /*value_ptr*/ NULL);
    }

    free(private_data);
    free(threads);
  } else {
    fprintf(stderr, "Error: could not allocate %" PRIu64 " threads\n"
      , shared_data->thread_count);
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
  /**
   * @brief Convierte el puntero a uno de tipo private_data_t.
   */
  private_data_t* private_data = (private_data_t*) data;
  /**
   * @brief Accede a los datos compartidos utilizando el puntero.
   */
  shared_data_t* shared_data = private_data->shared_data;

  // print "Hello from secondary thread"
  /**
   * @brief Se accede a los miembros de las estructuras privada y compartida 
   * para imprimir el número del hilo actual y el número total de hilos.
  */
  printf("Hello from secondary thread %" PRIu64 " of %" PRIu64 "\n"
    , private_data->thread_number, shared_data->thread_count);
  return NULL;
}  // end procedure