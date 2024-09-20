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
  simulate(&plate, data->delta_t, data->alpha, data->h, data->epsilon, &k, &time_seconds);

  // Esperar hasta que sea el turno del hilo para escribir
  pthread_mutex_lock(&data->shared_data->write_mutex);
  while (data->shared_data->current_turn != data->thread_index) {
    pthread_cond_wait(&data->shared_data->turn_cond, &data->shared_data->write_mutex);
  }

  // Escribir reporte y matriz cuando sea el turno correcto
  create_report(data->job_file, data->plate_filename, data->delta_t, data->alpha,
    data->h, data->epsilon, k, time_seconds, data->output_dir);

  char output_filename[256];
  snprintf(output_filename, sizeof(output_filename), "%s/plate%03d-%d.bin",
    data->output_dir, atoi(&data->plate_filename[5]), k);
  write_plate(output_filename, &plate);

  // Incrementar el turno y despertar a los demás hilos
  data->shared_data->current_turn++;
  pthread_cond_broadcast(&data->shared_data->turn_cond);
  pthread_mutex_unlock(&data->shared_data->write_mutex);

  // Liberar memoria
  for (long long int i = 0; i < plate.rows; i++) {
    free(plate.data[i]);
  }
  free(plate.data);
  free(data);  // Liberar la estructura data
  pthread_exit(NULL);
}

// Función para contar las líneas de trabajo en el archivo de trabajo
int count_job_lines(const char* job_file) {
  FILE* file = fopen(job_file, "r");
  if (file == NULL) {
    perror("Error opening the job file");
    return -1;
  }
  int count = 0;
  char line[1024];
  while (fgets(line, sizeof(line), file)) {
    count++;
  }
  fclose(file);
  return count;
}

int main(int argc, char *argv[]) {
  if (argc < 3 || argc > 5) {
    fprintf(stderr,
      "Usage: %s <job file> [<thread count>] <input dir> <output dir>\n",
        argv[0]);
    return EXIT_FAILURE;
  }

  const char* job_file = argv[1];

  // Si no se ingresan hilos, usar el número de CPUs disponibles
  int num_threads;
  if (argc >= 4 && isdigit(argv[2][0])) { // Verificamos si el argumento de hilos es válido
    num_threads = atoi(argv[2]);
    if (num_threads <= 0) {
      num_threads = sysconf(_SC_NPROCESSORS_ONLN);
      if (num_threads <= 0) {
        perror("Error detecting number of CPUs");
        return EXIT_FAILURE;
      }
      else {
        printf("Invalid thread count. Using %d threads (detected from CPUs)\n", num_threads);
      }
    }
   } else {
    // Detectar número de procesadores si no se especifica
    num_threads = sysconf(_SC_NPROCESSORS_ONLN);
    if (num_threads <= 0) {
      perror("Error detecting number of CPUs");
      return EXIT_FAILURE;
    }
    printf("Using %d threads (detected from CPUs)\n", num_threads);
  }

  // Verificar que los directorios estén bien especificados
  const char* input_dir;
  const char* output_dir;
  
  if (argc == 3) {  // No se especificó el número de hilos, input_dir ni output_dir
    input_dir = ".";
    output_dir = ".";
  } else if (argc == 4) {  // Se especificó solo input_dir y output_dir
    input_dir = argv[2];
    output_dir = argv[3];
  } else if (argc == 5) {  // Se especificaron thread count, input_dir y output_dir
    input_dir = argv[argc - 2];
    output_dir = argv[argc - 1];
  } else {
    input_dir = ".";
    output_dir = ".";
  }

  // Crear el directorio de salida si no existe
  struct stat st = {0};
  if (stat(output_dir, &st) == -1) {
    if (mkdir(output_dir, 0700) != 0) {
      perror("Error creating output directory");
      return EXIT_FAILURE;
    }
  }

  // Contar la cantidad total de trabajos en el archivo
  int total_jobs = count_job_lines(job_file);
  if (total_jobs < 0) {
    return EXIT_FAILURE;  // Error al contar las líneas
  }

  // Ajustar el número de hilos al número de trabajos si es necesario
  if (num_threads > total_jobs) {
    num_threads = total_jobs;
  }

  FILE* file = fopen(job_file, "r");
  if (file == NULL) {
    perror("Error opening the job file");
    return EXIT_FAILURE;
  }

  // Inicializar SharedData
  SharedData shared_data;
  pthread_mutex_init(&shared_data.write_mutex, NULL);
  pthread_cond_init(&shared_data.turn_cond, NULL);
  shared_data.current_turn = 0;  // Inicializamos en 0 para que el primer hilo sea el 0

  pthread_t threads[num_threads];
  int thread_count = 0;  // Contador de hilos en ejecución
  char plate_filename[MAX_PATH_LENGTH];
  double delta_t, alpha, h, epsilon;
  int job_index = 0;

  while (fscanf(file, "%s %lf %lf %lf %lf", plate_filename, &delta_t, &alpha, &h, &epsilon) == 5) {
    // Crear la estructura SimulationData para pasarle los datos a cada hilo
    SimulationData* data = (SimulationData*) malloc(sizeof(SimulationData));

    int needed_size = snprintf(NULL, 0, "%s/%s", input_dir, plate_filename) + 1; // +1 para el terminador nulo

    if (needed_size > sizeof(data->input_filepath)) {
      fprintf(stderr, "Error: the combined file path is too long\n");
      free(data);  // Liberar la memoria asignada para evitar fugas
      pthread_exit(NULL);  // O manejar el error apropiadamente
    }

    snprintf(data->input_filepath, sizeof(data->input_filepath), "%s/%s", input_dir, plate_filename);
    strcpy(data->plate_filename, plate_filename);
    data->delta_t = delta_t;
    data->alpha = alpha;
    data->h = h;
    data->epsilon = epsilon;
    data->job_file = job_file;
    data->output_dir = output_dir;
    data->thread_index = job_index;  // Asignar el índice del hilo
    data->shared_data = &shared_data;  // Pasar el puntero a SharedData

    // Crear el hilo
    if (pthread_create(&threads[thread_count], NULL, process_simulation, (void*) data) != 0) {
      perror("Error creating thread");
      return EXIT_FAILURE;
    }

    thread_count++;
    job_index++;

    // Si hemos alcanzado el número máximo de hilos permitidos, esperamos que terminen
    if (thread_count == num_threads || job_index == total_jobs) {
      // Esperar a que todos los hilos del lote actual terminen
      for (int i = 0; i < thread_count; i++) {
        pthread_join(threads[i], NULL);
      }
      thread_count = 0;  // Reiniciar el contador de hilos para el siguiente lote
    }
  }

  fclose(file);

  // Destruir los mutex y condiciones
  pthread_mutex_destroy(&shared_data.write_mutex);
  pthread_cond_destroy(&shared_data.turn_cond);

  return EXIT_SUCCESS;
}