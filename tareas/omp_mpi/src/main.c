// Copyright 2024 Josue Torres Sibaja <josue.torressibaja@ucr.ac.cr>

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
  double start_time = omp_get_wtime();  ///< OpenMP timing.

  // Verify command line arguments.
  if (argc < 4 || argc > 5) {
    fprintf(stderr, "Usage: <job file> <input dir> <output dir> "
      "<thread_count>\n");
    return 11;
  }
  const char* job_filename = argv[1];
  const char* input_dir = argv[2];
  const char* output_dir = argv[3];

  // Configure thread count.
  uint64_t thread_count = sysconf(_SC_NPROCESSORS_ONLN);
  if (argc == 5) {
    if (sscanf(argv[4], "%" SCNu64, &thread_count) != 1) {
      fprintf(stderr, "Invalid thread count.\n");
      return 12;
    }
  }

  // Set OpenMP thread count.
  omp_set_num_threads(thread_count);

  // Extract job number from filename.
  uint64_t job_num = 0;
  sscanf(job_filename, "job%03lu.txt", &job_num);

  // Create paths for input/output files.
  char filepath[150];
  snprintf(filepath, sizeof(filepath), "%s", output_dir);
  char txt_path[255];
  snprintf(txt_path, sizeof(txt_path), "%s/%s", input_dir, job_filename);
  char report_path[255];
  snprintf(report_path, sizeof(report_path), "%s/job%03lu.tsv", filepath,
    job_num);

  // Create report file.
  FILE* report_file = fopen(report_path, "w");
  if (!report_file) {
    perror("Error opening report file.");
    return 1;
  }
  fclose(report_file);

  // Read simulation parameters.
  uint64_t struct_count = 0;
  SimData* simulation_parameters = read_job_file(txt_path, &struct_count);
  if (simulation_parameters == NULL) {
    fprintf(stderr, "Error reading job file.\n");
    return 1;
  }

  // Run simulation.
  const char* plate_filename;
  for (uint64_t i = 0; i < struct_count; i++) {
    plate_filename = simulation_parameters[i].bin_name;
    configure_simulation(plate_filename, simulation_parameters[i], report_path,
      input_dir, thread_count);
  }

  // Calculate elapsed time using OpenMP timing.
  double end_time = omp_get_wtime();
  double elapsed_secs = end_time - start_time;
  double elapsed_ns = elapsed_secs * 1e9;

  printf("Execution time (seconds): %.9lf\n", elapsed_secs);
  printf("Execution time (nanoseconds): %.9lf\n", elapsed_ns);

  // Free memory.
  free(simulation_parameters);
  return 0;
}
