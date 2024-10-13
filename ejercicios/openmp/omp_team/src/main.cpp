/**
 * @file hello_omp.cpp
 * @brief Programa que utiliza OpenMP para crear hilos y mostrar un mensaje
 * desde cada hilo.
 *
 * @details Este programa genera hilos en paralelo utilizando OpenMP. El número
 * de hilos puede ser especificado como un argumento de línea de comandos o, si
 * no se proporciona, se usa el número de procesadores disponibles en el
 * sistema. Cada hilo imprime su número de identificación y la cantidad total
 * de hilos.
 *
 * @author ECCI-UCR
 * @copyright CC-BY 4.0 2024
 */

#include <iostream>
#include <omp.h>

/**
 * @brief Función principal que genera hilos y muestra mensajes desde cada uno.
 *
 * @param argc Número de argumentos de línea de comandos.
 * @param argv Array de punteros a los argumentos de línea de comandos. El
 * primer argumento puede ser usado para especificar el número de hilos.
 *
 * @return int 0 si la ejecución fue exitosa.
 */
int main(int argc, char* argv[]) {
  /**
   * Determina el número de hilos: usa el argumento de línea de comandos si
   * está presente, de lo contrario, se asigna el número de procesadores
   * disponibles.
   */
  const int thread_count = (argc == 2) ? \
    std::atoi(argv[1]) : omp_get_num_procs();

  /** Establece el número de hilos que OpenMP utilizará. */
  omp_set_num_threads(thread_count);

  /** Crea los hilos en paralelo. */
  #pragma omp parallel default(none) shared(thread_count, std::cout)
  {
    /** Cada hilo obtiene su número de identificación único. */
    int thread_id = omp_get_thread_num();

    /**
     * La sección crítica asegura que solo un hilo a la vez imprima el
     * mensaje, evitando la mezcla de salidas entre hilos.
     */
    #pragma omp critical
    {
      std::cout << "Hello from secondary thread " << thread_id 
        << " of " << thread_count << std::endl;
    }
  }
  return 0;
}
