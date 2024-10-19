// Copyright 2024 Josué Torres Sibaja <josue.torressibaja@ucr.ac.cr>

#include "heat_simulation.h"  // NOLINT

/**
 * @file main.c
 * @brief Programa para realizar simulaciones de propagación de calor en una
 * lámina.
 * 
 * @details El programa lee las dimensiones y valores iniciales de temperatura
 * de una lámina desde un archivo binario, realiza una simulación de
 * propagación de calor para encontrar el momento de equilibro térmico, y
 * genera archivos de reporte y salida con los resultados. Los hilos ingresados
 * realizan operaciones sobre un determinado número de filas de la lámina.
 */

/**
 * @brief Función principal para ejecutar las simulaciones térmicas utilizando
 * múltiples hilos.
 * 
 * @details La función toma como entrada un archivo de trabajo que contiene las
 * especificaciones de varias simulaciones térmicas a realizar. Cada línea en
 * el archivo contiene información sobre un trabajo de simulación. El programa
 * puede utilizar una cantidad de hilos especificada por el usuario o detectar
 * el número de procesadores disponibles para determinar la cantidad de hilos a
 * crear. Se utiliza la cantidad de hilos ingresada para hacer operaciones
 * sobre la matriz, repartiendo el trabajo de forma equitativa entre ellos.
 * 
 * @param argc Cantidad de argumentos pasados por línea de comandos.
 * 
 * @param argv Argumentos de línea de comandos.
 * - 1: Número de hilos.
 * - 2: Archivo de trabajo que contiene los datos de la simulación.
 * - 3: Directorio de entrada donde se encuentran los archivos de placas.
 */
int main(int argc, char* argv[]) {
  /** Tomar el tiempo de inicio. */
  struct timespec start_time, finish_time;
  clock_gettime(/*clk_id*/CLOCK_MONOTONIC, &start_time);

  const char* job_name = argv[2];
  const char* dir = argv[3];
  uint64_t thread_count = sysconf(_SC_NPROCESSORS_ONLN);
  if (argc < 4) {
    fprintf(stderr, "Usage: [thread_count] <job_file.txt> <input_dir>\n");
  } else if (argc == 4) {
    if (sscanf(argv[1], "%" SCNu64, & thread_count) != 1) {
      fprintf(stderr, "Error: invalid thread count\n");
      return 11;
    }
    if (thread_count == 0) {
      thread_count = sysconf(_SC_NPROCESSORS_ONLN);
    }
    uint64_t lines = 0;
    SimData* sim_params = read_job_file(job_name, dir, &lines);
    configure_simulation(dir, sim_params, lines, job_name, thread_count);
    for (uint64_t i = 0; i < lines; i++) {
      if (sim_params[i].bin_name != NULL) {
        free(sim_params[i].bin_name);
      }
    }
    free(sim_params);
  }

  /** Tomar el tiempo de finalización. */
  clock_gettime(/*clk_id*/CLOCK_MONOTONIC, &finish_time);

  /** Calcular el tiempo transcurrido en segundos y nanosegundos. */
  double elapsed_secs = (finish_time.tv_sec - start_time.tv_sec) +
    (finish_time.tv_nsec - start_time.tv_nsec) * 1e-9;
  double elapsed_ns = (finish_time.tv_sec - start_time.tv_sec) * 1e9 +
    (finish_time.tv_nsec - start_time.tv_nsec);

  printf("Execution time (seconds): %.9lf\n", elapsed_secs);
  printf("Execution time (nanoseconds): %.9lf\n", elapsed_ns);

  return 0;
}
