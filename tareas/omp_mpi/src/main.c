// Copyright 2024 Josué Torres Sibaja <josue.torressibaja@ucr.ac.cr>

#include "heat_simulation.h"

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
 * @param argv Argumentos de línea de comandos.
 */
int main(int argc, char *argv[]) {
  /** Tomar el tiempo de inicio. */
  struct timespec start_time, finish_time;
  clock_gettime(/*clk_id*/CLOCK_MONOTONIC, &start_time);

  /**
   * Verificar que los argumentos proporcionados en la línea de comandos sean
   * correctos.
   */
  if (argc < 4 || argc > 5) {
    /**
     * Si se quiere compilar el programa con el Makefile en el directorio raíz
     * "pthread_optimized", se debe correr el programa de la siguiente forma:
     * bin/pthread_optimized job001.txt test/job001/input test/job001/output 4
     *
     * Los "job" pueden ser reemplazados por el número de job que se desee.
     */
    fprintf(stderr, "Usage: <job file> <input dir> <output dir> "
      "<thread_count>\n");
    return 11;
  }
  const char* job_filename = argv[1];
  const char* input_dir = argv[2];
  const char* output_dir = argv[3];

  /** Configurar cantidad de hilos. */
  uint64_t thread_count = sysconf(_SC_NPROCESSORS_ONLN);
  if (argc == 5) {
    if (sscanf(argv[4], "%" SCNu64, &thread_count) == 1) {
    } else {
      fprintf(stderr, "Invalid thread count.\n");
      return 12;
    }
  }

  uint64_t job_num = 0;
  sscanf(job_filename, "job%03lu.txt", &job_num);

  /** Crear la ruta para los archivos de entrada. */
  char filepath[150];
  snprintf(filepath, sizeof(filepath), "%s", output_dir);
  char txt_path[255];
  snprintf(txt_path, sizeof(txt_path), "%s/%s", input_dir, job_filename);
  char report_path[255];
  snprintf(report_path, sizeof(report_path), "%s/job%03lu.tsv", filepath,
    job_num);
  FILE* report_file = fopen(report_path, "w");
  if (!report_file) {
    perror("Error opening report file.\n");
    return 1;
  }
  fclose(report_file);

  uint64_t struct_count = 0;
  SimData* simulation_parameters = read_job_file(txt_path, &struct_count);
  if (simulation_parameters == NULL) {
    fprintf(stderr, "Error reading job file.\n");
    return 1;
  }

  const char* plate_filename;
  for (uint64_t i = 0; i < struct_count; i++) {
    plate_filename = simulation_parameters[i].bin_name;
    /** Ejecutar la simulación. */
    configure_simulation(plate_filename, simulation_parameters[i], report_path,
      input_dir, thread_count);
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

  /** Liberar memoria. */
  free(simulation_parameters);
  return 0;
}
