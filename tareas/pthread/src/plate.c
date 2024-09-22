// Copyright 2024 Josué Torres Sibaja <josue.torressibaja@ucr.ac.cr>

#include "plate.h"  // NOLINT

SimulationData* read_job_file(const char* job_file, const char* dir,
  uint64_t* job_lines) {
  FILE *file;
  SimulationData* sim_params;
  char filepath[MAX_PATH_LENGTH];

  // Construye la ruta completa del archivo usando el directorio y el nombre
  snprintf(filepath, sizeof(filepath), "%s/%s", dir, job_file);

  // Cuenta el número de líneas en el archivo de trabajos
  *job_lines = count_job_lines(filepath);

  // Asigna memoria para un arreglo de estructuras SimulationData
  sim_params = malloc(*job_lines * sizeof(SimulationData));
  if (sim_params == NULL) {
    fprintf(stderr, "Error allocating memory for simulation parameters\n");
    return NULL;  // Devuelve NULL en caso de fallo
  }

  // Abre el archivo de trabajos para leer los datos
  file = fopen(filepath, "r");
  if (file == NULL) {
    perror("Error opening the job file\n");
    free(sim_params);  // Libera la memoria si no se puede abrir el archivo
    return NULL;  // Devuelve NULL si no se puede abrir el archivo
  }

  int i = 0;
  char plate_name[MAX_PATH_LENGTH];

  // Extrae el nombre del archivo y los parámetros de cada línea
  while (fscanf(file, "%s %lf %lf %lf %lf", plate_name,
    &sim_params[i].delta_t, &sim_params[i].alpha,
      &sim_params[i].h, &sim_params[i].epsilon) == 5) {
    // Asigna memoria para almacenar el nombre del archivo leído
    sim_params[i].filename = malloc(strlen(plate_name) + 1);
    if (sim_params[i].filename == NULL) {
      fprintf(stderr,
        "Error allocating memory for file name at line %d\n", i);
      // Libera toda la memoria asignada hasta ahora
      for (int j = 0; j < i; j++) {
        free(sim_params[j].filename);
      }
      free(sim_params);
      fclose(file);
      return NULL;  // Devuelve NULL en caso de error
    }
    // Copia el nombre del archivo en la estructura de datos
    snprintf(sim_params[i].filename, strlen(plate_name) + 1, "%s", plate_name);
    i++;
  }
  // Cierra el archivo de trabajos
  fclose(file);
  // Devuelve el arreglo de estructuras con los parámetros de simulación
  return sim_params;
}

void thread_simulation(SimulationData* sim_params, const char* job_file,
  const char* dir, uint64_t job_lines, uint64_t num_threads) {
  FILE* file;
  char filepath[MAX_PATH_LENGTH];
  SharedData* shared_data = malloc(sizeof(SharedData));
  if (shared_data == NULL) {
    fprintf(stderr, "Error allocating memory for shared data\n");
    return EXIT_FAILURE;
  }

  // Arreglo para almacenar los estados finales de cada simulación
  uint64_t* sim_states = malloc(job_lines * sizeof(uint64_t));
  if (sim_states == NULL) {
    fprintf(stderr, "Error allocating memory for simulation states\n");
    free(shared_data);  // Libera la memoria de shared_data antes de retornar
    return EXIT_FAILURE;
  }

  for (uint64_t job_index = 0; job_index < job_lines; job_index++) {
    // Construye la ruta completa al archivo binario de entrada
    snprintf(filepath, sizeof(filepath), "%s/%s", dir,
      sim_params[job_index].filename);

    // Abre el archivo binario
    file = fopen(filepath, "rb");
    if (file == NULL) {
      fprintf(stderr, "Error opening binary file %s\n",
        sim_params[job_index].filename);
      free(sim_states);
      free(shared_data);
      return EXIT_FAILURE;
    }

    // Lee el número de filas y columnas de la lámina
    fread(&(shared_data->rows), sizeof(uint64_t), 1, file);
    fread(&(shared_data->cols), sizeof(uint64_t), 1, file);

    // Asigna memoria para la matriz de la lámina
    shared_data->data = malloc(shared_data->rows * sizeof(double*));
    if (shared_data->data == NULL) {
      fprintf(stderr, "Error allocating memory for plate rows\n");
      fclose(file);
      free(sim_states);
      free(shared_data);
      return EXIT_FAILURE;
    }

    // Asigna memoria para cada fila de la matriz
    for (uint64_t row = 0; row < shared_data->rows; row++) {
      shared_data->data[row] = malloc(shared_data->cols * sizeof(double));
      if (shared_data->data[row] == NULL) {
        fprintf(stderr, "Error allocating memory for plate columns\n");
        // Liberación de memoria en caso de error
        for (uint64_t r = 0; r < row; r++) {
          free(shared_data->data[r]);
        }
        free(shared_data->data);
        fclose(file);
        free(sim_states);
        free(shared_data);
        return EXIT_FAILURE;
      }
    }

    // Lee los valores de la lámina del archivo binario
    for (uint64_t row = 0; row < shared_data->rows; row++) {
      if (fread(shared_data->data[row], sizeof(double), shared_data->cols,
        file) != shared_data->cols) {
        fprintf(stderr, "Error reading plate data of file %s\n",
          sim_params[job_index].filename);
        // Liberación de memoria en caso de error
        for (uint64_t r = 0; r < shared_data->rows; r++) {
          free(shared_data->data[r]);
        }
        free(shared_data->data);
        fclose(file);
        free(sim_states);
        free(shared_data);
        return EXIT_FAILURE;
      }
    }

    // Configura los parámetros de simulación en SharedData
    shared_data->delta_t = sim_params[job_index].delta_t;
    shared_data->alpha = sim_params[job_index].alpha;
    shared_data->h = sim_params[job_index].h;
    shared_data->epsilon = sim_params[job_index].epsilon;

    // Llama a la función de simulación
    uint64_t num_states = simulate(shared_data, num_threads);

    // Guarda el número de estados finales de la simulación
    sim_states[job_index] = num_states;

    // Genera el archivo binario con los resultados de la simulación
    write_plate(shared_data->data, shared_data->rows, shared_data->cols, dir,
      sim_params[job_index].filename, num_states);

    // Libera la memoria de la lámina
    for (uint64_t row = 0; row < shared_data->rows; row++) {
      free(shared_data->data[row]);
    }
    free(shared_data->data);
    // Cierra el archivo binario
    fclose(file);
  }

  // Genera el reporte en formato .tsv
  create_report(dir, job_file, sim_params, sim_states, job_lines);

  // Libera memoria
  free(sim_states);
  free(shared_data);
}
