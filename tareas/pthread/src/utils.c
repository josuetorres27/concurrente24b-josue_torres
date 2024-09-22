// Copyright 2024 Josué Torres Sibaja <josue.torressibaja@ucr.ac.cr>

#include "heat_sim.h"  // NOLINT

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

/** Función para contar las líneas en el archivo de trabajo. */
uint64_t count_job_lines(const char* job_file) {
  FILE* file = fopen(job_file, "r");
  if (file == NULL) {
    perror("Error opening the job file");
    return 0;  // Retornar 0 en caso de error
  }

  uint64_t count = 0;
  char line[1024];

  // Leer cada línea del archivo
  while (fgets(line, sizeof(line), file)) {
    // Si la línea no está vacía
    if (line[0] != '\n') {
      count++;
    }
  }
  fclose(file);
  return count;
}

/**
 * @brief Formatea el tiempo transcurrido en segundos.
 * 
 * @details El formato de salida es: YYYY/MM/DD hh:mm:ss.
 * 
 * @param seconds Tiempo en segundos.
 * @param text Cadena de caracteres donde se guarda el tiempo formateado.
 * @param capacity Capacidad de la cadena de caracteres.
 * @return Puntero a la cadena de caracteres formateada.
 */
char* format_time(const time_t seconds, char* text, const size_t capacity) {
  const struct tm* gmt = gmtime(&seconds);  // NOLINT
  snprintf(text, capacity, "%04d/%02d/%02d\t%02d:%02d:%02d",
    gmt->tm_year - 70, gmt->tm_mon, gmt->tm_mday - 1, gmt->tm_hour,
      gmt->tm_min, gmt->tm_sec);
  return text;
}
