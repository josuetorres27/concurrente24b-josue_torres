// Copyright 2024 Josue Torres Sibaja <josue.torressibaja@ucr.ac.cr>

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
  int rank, size;
  double start_time, end_time;

  // Initialize MPI.
  MPI_Init(&argc, &argv);
  MPI_Comm_rank(MPI_COMM_WORLD, &rank);
  MPI_Comm_size(MPI_COMM_WORLD, &size);

  // Only root process handles command line arguments and file operations.
  if (rank == 0) {
    if (argc != 4) {
      fprintf(stderr, "Usage: <job file> <input dir> <output dir>\n");
      MPI_Abort(MPI_COMM_WORLD, 11);
      return 11;
    }
  }

  // Broadcast command line arguments to all processes.
  char job_filename[256], input_dir[256], output_dir[256];
  if (rank == 0) {
    strncpy(job_filename, argv[1], sizeof(job_filename) - 1);
    strncpy(input_dir, argv[2], sizeof(input_dir) - 1);
    strncpy(output_dir, argv[3], sizeof(output_dir) - 1);
  }
  MPI_Bcast(job_filename, 256, MPI_CHAR, 0, MPI_COMM_WORLD);
  MPI_Bcast(input_dir, 256, MPI_CHAR, 0, MPI_COMM_WORLD);
  MPI_Bcast(output_dir, 256, MPI_CHAR, 0, MPI_COMM_WORLD);

  // Start timing.
  start_time = MPI_Wtime();

  uint64_t job_num = 0;
  if (rank == 0) {
    sscanf(job_filename, "job%03lu.txt", &job_num);

    // Create output directories and report file.
    char filepath[150];
    snprintf(filepath, sizeof(filepath), "%s", output_dir);
    char report_path[255];
    snprintf(report_path, sizeof(report_path), "%s/job%03lu.tsv", filepath,
      job_num);
    FILE* report_file = fopen(report_path, "w");
    if (!report_file) {
      perror("Error opening report file.\n");
      MPI_Abort(MPI_COMM_WORLD, 1);
      return 1;
    }
    fclose(report_file);
  }

  // Read job file and broadcast simulation parameters.
  uint64_t struct_count = 0;
  SimData* simulation_parameters = NULL;

  if (rank == 0) {
    char txt_path[255];
    snprintf(txt_path, sizeof(txt_path), "%s/%s", input_dir, job_filename);
    simulation_parameters = read_job_file(txt_path, &struct_count);
    if (simulation_parameters == NULL) {
      fprintf(stderr, "Error reading job file.\n");
      MPI_Abort(MPI_COMM_WORLD, 1);
      return 1;
    }
  }

  // Broadcast struct_count to all processes.
  MPI_Bcast(&struct_count, 1, MPI_UNSIGNED_LONG_LONG, 0, MPI_COMM_WORLD);

  // Allocate memory for simulation parameters on other processes.
  if (rank != 0) {
    simulation_parameters = (SimData*)malloc(struct_count * sizeof(SimData));
  }

  // Broadcast simulation parameters to all processes.
  for (uint64_t i = 0; i < struct_count; i++) {
    MPI_Bcast(&simulation_parameters[i], sizeof(SimData), MPI_BYTE, 0,
      MPI_COMM_WORLD);
  }

  // Process simulations.
  char report_path[255];
  if (rank == 0) {
    snprintf(report_path, sizeof(report_path), "%s/job%03lu.tsv", output_dir,
      job_num);
  }

  for (uint64_t i = 0; i < struct_count; i++) {
    const char* plate_filename = simulation_parameters[i].bin_name;
    configure_simulation(plate_filename, simulation_parameters[i],
      report_path, input_dir, output_dir);

    // Synchronize processes after each simulation.
    MPI_Barrier(MPI_COMM_WORLD);
  }

  // End timing.
  end_time = MPI_Wtime();

  // Only root process prints timing information.
  if (rank == 0) {
    double elapsed_secs = end_time - start_time;
    printf("MPI Execution time (seconds): %.9lf\n", elapsed_secs);
    printf("Number of processes: %d\n", size);
  }

  // Clean up.
  free(simulation_parameters);
  MPI_Finalize();
  return 0;
}
