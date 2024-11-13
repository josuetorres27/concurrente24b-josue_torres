// Copyright 2024 Josué Torres Sibaja <josue.torressibaja@ucr.ac.cr>

/**
 * @file main.c
 * @brief Programa para realizar simulaciones de propagación de calor en una
 * lámina.
 *
 * @details El programa lee las dimensiones y valores iniciales de temperatura
 * de una lámina desde un archivo binario, realiza una simulación de
 * propagación de calor para encontrar el momento de equilibro térmico, y
 * genera archivos de salida con los resultados.
 */

#include "plate.h"

/**
 * @brief Función principal del programa.
 *
 * @details Lee el archivo de trabajo especificado, realiza la simulación de
 * propagación de calor para cada lámina, y genera archivos de reporte y de
 * salida con los resultados.
 *
 * @param argc Número de argumentos de la línea de comandos.
 * @param argv Arreglo de argumentos de la línea de comandos.
 * @return Código de retorno del programa.
 */
int main(int argc, char* argv[]) {
  /** Tomar el tiempo de inicio. */
  struct timespec start_time, finish_time;
  clock_gettime(/*clk_id*/CLOCK_MONOTONIC, &start_time);

  /**
   * Verificar que los argumentos proporcionados en la línea de comandos sean
   * correctos.
   */
  if (argc < 4 || argc > 4) {
    /**
     * Si se quiere compilar el programa con el Makefile en el directorio raíz
     * "serial_optimized", se debe correr el programa de la siguiente forma:
     * bin/serial_optimized job001.txt test/job001/input test/job001/output
     *
     * Los "job" pueden ser reemplazados por el número de job que se desee.
     */
    fprintf(stderr, "Usage: <job file> <input dir> <output dir>\n");
    return 11;
  }
  const char* job_filename = argv[1];
  const char* input_dir = argv[2];
  const char* output_dir = argv[3];

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
      input_dir, output_dir);
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
