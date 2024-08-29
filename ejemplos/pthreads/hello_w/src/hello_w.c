// Copyright 2021 Jeisson Hidalgo <jeisson.hidalgo@ucr.ac.cr> CC-BY 4.0

/**
 * @file hello_w.c
 * @brief Se crean n hilos de ejecución, donde cada uno saluda e indica su 
 * rank, y al final se saluda desde el hilo principal.
 * 
 * @details El programa crea una cantidad arbitraria de hilos de ejecución. 
 * Cada hilo imprime "Hello from secondary thread i" en la salida estándar 
 * y termina su ejecución. El usuario debe indicar la cantidad de hilos de 
 * ejecución como argumento de la línea de comandos.
 */

#include <assert.h>
#include <inttypes.h>
#include <pthread.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h> /**< Trae cosas de la biblioteca estándar de Unix */

void* greet(void* data);
int create_threads(uint64_t thread_count);

// procedure main(argc, argv[])
int main(int argc, char* argv[]) { /**< Se reciben los argumentos que se mandan por consola y crea un arreglo de caracteres */
#if 0
  for (int index = 0; index < argc; ++index) {
    printf("argv[%d] = '%s'\n", index, argv[index]);
  }
  return 0;
#endif

  int error = EXIT_SUCCESS;
  // create thread_count as result of converting argv[1] to integer
  // thread_count := integer(argv[1])
  uint64_t thread_count = sysconf(_SC_NPROCESSORS_ONLN); /**< Se crea un entero sin signo de 64 bits para especificar el espacio que se necesita*/
  if (argc == 2) { /**< Se reciben dos argumentos, los cuales deben ser utilizados*/
    if (sscanf(argv[1], "%" SCNu64, &thread_count) == 1) { /**< Se hace sscanf() para convertir texto (caracteres) a un entero de 64 bits, luego se guarda en thread_count*/
    } else {
      fprintf(stderr, "Error: invalid thread count\n"); /**< Se imprime si no se logra hacer el cast de antes*/
      return 11;
    }
  }

  error = create_threads(thread_count); /**< error va a indicar si hubo algún error en alguna de las creaciones de los hilos*/
  return error;
}  // end procedure

/**
 * @brief Crea una cantidad especificada de hilos
 * 
 * Esta función recibe dos enteros y retorna su suma.
 * 
 * @param[in] a Primer número a sumar.
 * @param[in] b Segundo número a sumar.
 * @return La suma de los dos números.
 */
int create_threads(uint64_t thread_count) {
  int error = EXIT_SUCCESS;
  // for thread_number := 0 to thread_count do
  pthread_t* threads = (pthread_t*) malloc(thread_count * sizeof(pthread_t)); /**< Se crea un arreglo (puntero) de estructuras de tipo pthread_t, el cual llama al sistema operativo para solicitar memoria para cada una de las estructuras*/
  if (threads) {
    for (uint64_t thread_number = 0; thread_number < thread_count
        ; ++thread_number) { /**< Este for va recorriendo cada una de las estructuras pthread_t que se crearon en el arreglo*/
      // create_thread(greet, thread_number)
      error = pthread_create(&threads[thread_number], /*attr*/ NULL, greet
        , /*arg*/ (void*) thread_number); /**< Para cada uno de los pthreads se hace un pthread_create() desde el hilo principal*/
      if (error == EXIT_SUCCESS) {
      } else {
        fprintf(stderr, "Error: could not create secondary thread\n");
        error = 21;
        break;
      }
    }

    // print "Hello from main thread"
    printf("Hello from main thread\n"); /**< El hilo principal saluda una vez que se han creado todos los hilos secundarios*/

    for (uint64_t thread_number = 0; thread_number < thread_count
        ; ++thread_number) {
      pthread_join(threads[thread_number], /*value_ptr*/ NULL); /**< Se recorre la estructura threads nuevamente para hacer que el hilo principal espere a cada hilo secundario*/
    }

    free(threads); /**< Se libera la memoria utilizada para el arreglo de hilos*/
  } else {
    fprintf(stderr, "Error: could not allocate %" PRIu64 " threads\n"
      , thread_count);
    error = 22;
  }

  return error;
}

/**
 * @brief Suma dos números enteros.
 * 
 * Esta función recibe dos enteros y retorna su suma.
 * 
 * @param[in] a Primer número a sumar.
 * @param[in] b Segundo número a sumar.
 * @return La suma de los dos números.
 */
// procedure greet:
void* greet(void* data) {
  // assert(data);
  const uint64_t rank = (uint64_t) data; /**< rank es el número identificador de cada hilo, el cual es el argumento recibido a la hora de hacer pthread_create */
  // print "Hello from secondary thread"
  printf("Hello from secondary thread %" PRIu64 "\n", rank);
  return NULL;
}  // end procedure