// Copyright 2021 Jeisson Hidalgo <jeisson.hidalgo@ucr.ac.cr> CC-BY 4.0

/**
 * @file hello_w.c
 * @brief Crea n cantidad de hilos secundarios, donde cada uno saluda e indica 
 * su rank. Al final saluda desde el hilo principal.
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
#include <unistd.h> /**< Permite utilizar características de la biblioteca estándar de Unix. */

void* greet(void* data);
int create_threads(uint64_t thread_count);

/**
 * @brief Crea un arreglo con una cantidad de hilos secundarios que recibe por parámetro.
 * 
 * @param[in] argc Número de argumentos de línea de comandos.
 * @param[in] argv Arreglo de cadenas de caracteres que representan los argumentos 
 * de línea de comandos.
 * @return Código de estado del programa.
 */
int main(int argc, char* argv[]) { /**< Se reciben los argumentos que se mandan por consola y crea un arreglo de caracteres. */
  int error = EXIT_SUCCESS;
  uint64_t thread_count = sysconf(_SC_NPROCESSORS_ONLN); /**< Entero sin signo de 64 bits para especificar el espacio que se necesita. */
  if (argc == 2) {
    if (sscanf(argv[1], "%" SCNu64, &thread_count) == 1) { /**< Se convierte el texto a un entero de 64 bits y se guarda en thread_count. */
    } else {
      fprintf(stderr, "Error: invalid thread count\n"); /**< Se imprime si no se logra hacer el cast del texto. */
      return 11;
    }
  }
  error = create_threads(thread_count); /**< Indica si hubo algún error en las creaciones de los hilos. */
  return error;
}

/**
 * @brief Crea una cantidad especificada de hilos.
 * 
 * @param[in] thread_count Número de hilos secundarios a crear.
 * @return Código de estado.
 */
int create_threads(uint64_t thread_count) {
  int error = EXIT_SUCCESS;
/**
 * @brief Crea un arreglo (puntero) de estructuras de tipo pthread_t, el cual 
 * llama al SO para solicitar memoria para cada una de las estructuras.
 */
  pthread_t* threads = (pthread_t*) malloc(thread_count * sizeof(pthread_t));
  if (threads) {
    for (uint64_t thread_number = 0; thread_number < thread_count
        ; ++thread_number) { /**< Va recorriendo cada una de las estructuras pthread_t que se crearon en el arreglo. */
      error = pthread_create(&threads[thread_number], /*attr*/ NULL, greet
        , /*arg*/ (void*) thread_number); /**< Para cada uno de los pthread_t se hace un pthread_create() desde el hilo principal. */
      if (error == EXIT_SUCCESS) {
      } else {
        fprintf(stderr, "Error: could not create secondary thread\n");
        error = 21;
        break;
      }
    }
    printf("Hello from main thread\n"); /**< El hilo principal saluda una vez que se han creado todos los hilos secundarios. */
    for (uint64_t thread_number = 0; thread_number < thread_count
        ; ++thread_number) {
      pthread_join(threads[thread_number], /*value_ptr*/ NULL); /**< Se recorre el arreglo para hacer que el hilo principal espere a cada hilo secundario. */
    }
    free(threads); /**< Se libera la memoria utilizada para el arreglo de hilos. */
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
 * @param[in] data Puntero a un valor que representa el rank del hilo.
 * @return Siempre retorna NULL.
 */
void* greet(void* data) {
  const uint64_t rank = (uint64_t) data; /**< rank es el número identificador de cada hilo, el cual es el argumento recibido a la hora de hacer pthread_create. */
  printf("Hello from secondary thread %" PRIu64 "\n", rank);
  return NULL;
}