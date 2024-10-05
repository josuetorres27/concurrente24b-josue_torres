// Copyright 2021 Allan Berrocal <allan.berrocal@ucr.ac.cr> and
// Jeisson Hidalgo <jeisson.hidalgo@ucr.ac.cr> CC-BY 4.0.
// Adapted from Mr. Jainam M https://github.com/MJjainam/falseSharing

#define _DEFAULT_SOURCE

#include <assert.h>
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

// Número de elementos en el array.
const size_t ELEMENT_COUNT = 1000;
// Índice del primer elemento.
const size_t FIRST_ELEMENT = 0;
// Índice del segundo elemento.
const size_t SECOND_ELEMENT = 1;
// Índice del último elemento.
const size_t LAST_ELEMENT = ELEMENT_COUNT - 1;
// Número de iteraciones por elemento.
const size_t ITERATION_COUNT = 100000000;
// Descripción de las diferentes ejecuciones.
const char* description[] = {
  "sequential(first, second)",
  "sequential(first, last)",
  "concurrent(first, second)",
  "concurrent(first, last)",
};
// Array global.
int* array = NULL;

/// @brief Actualiza un elemento del array de forma no eficiente/mejorable.
/// @param data Índice del elemento que se va a actualizar.
void *update_element(void *data) {
  const size_t my_index = (size_t) data;

  for (size_t iteration = 0; iteration < ITERATION_COUNT; ++iteration) {
    array[my_index] += iteration % ELEMENT_COUNT;
  }

  return NULL;
}

/// @brief Ejecuta actualizaciones secuenciales en dos índices del array.
/// @param index0 Índice del primer elemento a actualizar.
/// @param index1 Índice del segundo elemento a actualizar.
void run_sequential(size_t index0, size_t index1) {
  update_element((void*) index0);
  update_element((void*) index1);
}

/// @brief Ejecuta actualizaciones concurrentes en dos índices del array.
/// @param index0 Índice del primer elemento a actualizar.
/// @param index1 Índice del segundo elemento a actualizar.
void run_concurrent(size_t index0, size_t index1) {
  // Crea los hilos para actualizar los elementos en el arreglo.
  pthread_t thread0, thread1;
  pthread_create(&thread0, NULL, update_element, (void*) index0);
  pthread_create(&thread1, NULL, update_element, (void*) index1);
  // Espera a que los hilos terminen.
  pthread_join(thread0, NULL);
  pthread_join(thread1, NULL);
}

/// @brief Función principal que ejecuta el programa.
/// @param argc Número de argumentos de la línea de comandos.
/// @param argv Arreglo de argumentos de la línea de comandos.
/// @return Código de salida del programa.
int main(int argc, char* argv[]) {
  // Se obtiene el modo de ejecución de la línea de comandos.
  const int mode = argc >= 2 ? atoi(argv[1]) : 0;

  // Se reserva memoria dinámica para el array de tamaño ELEMENT_COUNT.
  array = (int*) calloc(ELEMENT_COUNT, sizeof(int));
  // Se asegura que la asignación de memoria fue exitosa.
  assert(array);

  // Se definen dos variables para almacenar los tiempos de inicio y fin.
  struct timespec start_time, finish_time;
  // Se registra el tiempo de inicio antes de la ejecución.
  clock_gettime(CLOCK_REALTIME, &start_time);

  // Dependiendo del valor de "mode", se ejecuta una de las cuatro opciones:
  // 0: Actualización secuencial de los primeros dos elementos.
  // 1: Actualización secuencial del primer y último elemento.
  // 2: Actualización concurrente de los primeros dos elementos.
  // 3: Actualización concurrente del primer y último elemento.
  switch (mode) {
    case 0: run_sequential(FIRST_ELEMENT, SECOND_ELEMENT); break;
    case 1: run_sequential(FIRST_ELEMENT, LAST_ELEMENT); break;
    case 2: run_concurrent(FIRST_ELEMENT, SECOND_ELEMENT); break;
    case 3: run_concurrent(FIRST_ELEMENT, LAST_ELEMENT); break;
  }

  // Se registra el tiempo de finalización después de la ejecución.
  clock_gettime(CLOCK_REALTIME, &finish_time);
  // Se calcula el tiempo transcurrido en segundos, tomando en cuenta la
  // diferencia entre los valores de segundos y nanosegundos de las dos marcas
  // de tiempo.
  double elapsed_time = finish_time.tv_sec - start_time.tv_sec +
    (finish_time.tv_nsec - start_time.tv_nsec) * 1e-9;

  // Si el modo está entre 0 y 3, se imprime el tiempo transcurrido en
  // milisegundos y una breve descripción de la ejecución realizada.
  if (mode >= 0 && mode < 4) {
    printf("%.6lfs: %s\n", 1000 * elapsed_time, description[mode]);
  }

  // Se libera la memoria asignada dinámicamente al array.
  free(array);

  return EXIT_SUCCESS;
}
