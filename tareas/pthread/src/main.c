// Copyright 2024 Josué Torres Sibaja <josue.torressibaja@ucr.ac.cr>

#include "plate.h"

/**
 * @file main.c
 * @brief Programa para realizar simulaciones de propagación de calor en una
 * lámina.
 * 
 * @details El programa lee las dimensiones y valores iniciales de temperatura
 * de una lámina desde un archivo binario, realiza una simulación de
 * propagación de calor para encontrar el momento de equilibro térmico, y
 * genera archivos de reporte y salida con los resultados.
 */

void* process_simulation(void* arg) {
  // Castear el argumento a la estructura correspondiente
  SimulationData* data = (SimulationData*) arg;

  Plate plate;
  if (read_dimensions(data->input_filepath, &plate) != EXIT_SUCCESS) {
    pthread_exit(NULL);
  }
  if (read_plate(data->input_filepath, &plate) != EXIT_SUCCESS) {
    pthread_exit(NULL);
  }

  int k;
  time_t time_seconds;
  simulate(&plate, data->delta_t, data->alpha, data->h, data->epsilon, &k,
    &time_seconds);

  // Generar reporte
  create_report(data->job_file, data->plate_filename, data->delta_t,
    data->alpha, data->h, data->epsilon, k, time_seconds, data->output_dir);
  // Escribir la matriz final en el archivo
  char output_filename[MAX_PATH_LENGTH];
  snprintf(output_filename, sizeof(output_filename), "%s/plate%03d-%d.bin",
    data->output_dir, atoi(&data->plate_filename[5]), k);
  write_plate(output_filename, &plate);

  // Liberar memoria
  for (long long int i = 0; i < plate.rows; i++) {
    free(plate.data[i]);
  }
  free(plate.data);
  pthread_exit(NULL);
}

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
int main(int argc, char *argv[]) {
  if (argc < 3 || argc > 5) {
    fprintf(stderr,
      "Usage: %s <job file> <thread count> <input dir> <output dir>\n",
        argv[0]);
    return EXIT_FAILURE;
  }

  const char* job_file = argv[1];
  int num_threads = (argc >= 4) ? atoi(argv[2]) : 1;
  const char* input_dir = (argc >= 4) ? argv[argc - 2] : ".";
  const char* output_dir = (argc >= 5) ? argv[argc - 1] : ".";

  struct stat st = {0};
  if (stat(output_dir, &st) == -1) {
    if (mkdir(output_dir, 0700) != 0) {
      perror("Error creating output directory");
      return EXIT_FAILURE;
    }
  }

  FILE* file = fopen(job_file, "r");
  if (file == NULL) {
    perror("Error opening the job file");
    return EXIT_FAILURE;
  }

  SharedData shared_data;
  pthread_mutex_init(&shared_data.write_mutex, NULL);
  shared_data.current_line = 0;

  char line[1024];  // Buffer para almacenar cada línea
  pthread_t threads[num_threads];
  int thread_count = 0;
  // Para almacenar el nombre del archivo de la placa
  char plate_filename[MAX_PATH_LENGTH];
  // Variables para los parámetros de la simulación
  double delta_t, alpha, h, epsilon;
  while (fscanf(file, "%s %lf %lf %lf %lf", plate_filename, &delta_t, &alpha,
    &h, &epsilon) == 5) {
    SimulationData* data = (SimulationData*) malloc(sizeof(SimulationData));
    int written = snprintf(data->input_filepath, sizeof(data->input_filepath),
      "%s/%s", input_dir, plate_filename);
    if (written < 0 || written >= (int) sizeof(data->input_filepath)) {
      fprintf(stderr, "Error: the file path is too long\n");
      pthread_exit(NULL);  // O un manejo adecuado de error
    }
    strcpy(data->plate_filename, plate_filename);
    data->delta_t = delta_t;
    data->alpha = alpha;
    data->h = h;
    data->epsilon = epsilon;
    data->job_file = job_file;
    data->output_dir = output_dir;

    if (pthread_create(&threads[thread_count], NULL, process_simulation,
      (void*) data) != 0) {
      perror("Error creating thread");
      return EXIT_FAILURE;
    }

    thread_count++;
    if (thread_count == num_threads) {
      // Esperar a que los hilos terminen
      for (int i = 0; i < thread_count; i++) {
        pthread_join(threads[i], NULL);
      }
      thread_count = 0;  // Reiniciar el contador de hilos
    }
  }

  // Esperar a que cualquier hilo restante termine
  for (int i = 0; i < thread_count; i++) {
    pthread_join(threads[i], NULL);
  }

  fclose(file);
  return EXIT_SUCCESS;
}
