// Copyright 2024 Josué Torres Sibaja <josue.torressibaja@ucr.ac.cr>

#include "plate.h"  // NOLINT

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

int main(int argc, char *argv[]) {
  if (argc < 3 || argc > 5) {
    fprintf(stderr,
      "Usage: %s <job file> [<thread count>] <input dir> <output dir>\n",
        argv[0]);
    return EXIT_FAILURE;
  }
  const char* job_file = argv[1];
  /** Si no se ingresan hilos, usar el número de CPUs disponibles. */
  int num_threads;
  /** Verificamos si el argumento de hilos es válido. */
  if (argc >= 4 && isdigit(argv[2][0])) {
    num_threads = atoi(argv[2]);
    if (num_threads <= 0) {
      /** Usar número de procesadores si el número de hilos es inválido. */
      num_threads = sysconf(_SC_NPROCESSORS_ONLN);
      if (num_threads <= 0) {
        perror("Error detecting number of CPUs");
        return EXIT_FAILURE;
      } else {
        printf("Invalid thread count. Using %d threads (detected from CPUs)\n",
          num_threads);
      }
    }
  } else {
    /** Detectar número de procesadores si no se especifica. */
    num_threads = sysconf(_SC_NPROCESSORS_ONLN);
    if (num_threads <= 0) {
      perror("Error detecting number of CPUs");
      return EXIT_FAILURE;
    }
    printf("Using %d threads (detected from CPUs)\n", num_threads);
  }
  /** Verificar que los directorios estén bien especificados. */
  const char* input_dir;
  const char* output_dir;
  /** No se especificó el número de hilos, input_dir ni output_dir. */
  if (argc == 3) {
    input_dir = ".";
    output_dir = ".";
  } else if (argc == 4) {  /** Se especificó solo input_dir y output_dir. */
    input_dir = argv[2];
    output_dir = argv[3];
  } else if (argc == 5) {  /** Se especificaron todos los argumentos. */
    input_dir = argv[argc - 2];
    output_dir = argv[argc - 1];
  } else {
    input_dir = ".";
    output_dir = ".";
  }
  /** Crear el directorio de salida si no existe. */
  struct stat st = {0};
  if (stat(output_dir, &st) == -1) {
    if (mkdir(output_dir, 0700) != 0) {
      perror("Error creating output directory");
      return EXIT_FAILURE;
    }
  }

  /** Contar la cantidad total de trabajos en el archivo. */
  int total_jobs = count_job_lines(job_file);
  if (total_jobs < 0) {
    return EXIT_FAILURE;  /** Error al contar las líneas. */
  }
  /** Ajustar el número de hilos al número de trabajos si es necesario. */
  if (num_threads > total_jobs) {
    num_threads = total_jobs;
  }

  FILE* file = fopen(job_file, "r");
  if (file == NULL) {
    perror("Error opening the job file");
    return EXIT_FAILURE;
  }

  /** Inicializar SharedData. */
  SharedData shared_data;
  pthread_mutex_init(&shared_data.write_mutex, NULL);
  pthread_cond_init(&shared_data.turn_cond, NULL);
  /** Inicializar el turno en 0 para que el primer hilo sea el 0. */
  shared_data.current_turn = 0;

  pthread_t threads[num_threads];  // NOLINT
  int thread_count = 0;  /** Contador de hilos en ejecución. */
  char plate_filename[MAX_PATH_LENGTH];
  double delta_t, alpha, h, epsilon;
  int job_index = 0;

  while (fscanf(file, "%s %lf %lf %lf %lf", plate_filename, &delta_t, &alpha,
    &h, &epsilon) == 5) {
    /** Crear la estructura para pasarle los datos a cada hilo. */
    SimulationData* data = (SimulationData*) malloc(sizeof(SimulationData));  // NOLINT

    /** Calcular el tamaño necesario para el filepath. */
    size_t needed_size = snprintf(NULL, 0, "%s/%s", input_dir, plate_filename)
      + 1;  /** +1 para el terminador nulo. */

    /** Verificar si el tamaño calculado es mayor que el tamaño del buffer. */
    if (needed_size > sizeof(data->input_filepath)) {
      fprintf(stderr, "Error: the combined file path is too long\n",
        needed_size, sizeof(data->input_filepath));
      free(data);  /** Liberar la memoria asignada para evitar fugas. */
      pthread_exit(NULL);  /** O manejar el error apropiadamente. */
    }

    /** Si el tamaño es adecuado, copiar la cadena. */
    snprintf(data->input_filepath, sizeof(data->input_filepath), "%s/%s",
      input_dir, plate_filename);

    strcpy(data->plate_filename, plate_filename);  // NOLINT
    data->delta_t = delta_t;
    data->alpha = alpha;
    data->h = h;
    data->epsilon = epsilon;
    data->job_file = job_file;
    data->output_dir = output_dir;
    data->thread_index = job_index;  /** Asignar el índice del hilo. */
    data->shared_data = &shared_data;  /** Pasar el puntero a SharedData. */

    /** Crear los hilos. */
    if (pthread_create(&threads[thread_count], NULL, process_simulation,
      (void*) data) != 0) {  // NOLINT
      perror("Error creating thread");
      return EXIT_FAILURE;
    }

    thread_count++;
    job_index++;

    /** Si se ha alcanzado el número máximo de hilos, espera a que terminen. */
    if (thread_count == num_threads || job_index == total_jobs) {
      /** Esperar a que todos los hilos del lote actual terminen. */
      for (int i = 0; i < thread_count; i++) {
        pthread_join(threads[i], NULL);
      }
      /** Reiniciar el contador de hilos para el siguiente lote. */
      thread_count = 0;
    }
  }

  fclose(file);
  /** Destruir los mutex y condiciones. */
  pthread_mutex_destroy(&shared_data.write_mutex);
  pthread_cond_destroy(&shared_data.turn_cond);

  return EXIT_SUCCESS;
}
