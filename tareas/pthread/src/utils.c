// Copyright 2024 Josué Torres Sibaja <josue.torressibaja@ucr.ac.cr>

#include "heat_simulation.h"  // NOLINT

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
  snprintf(text, capacity, "%04d/%02d/%02d\t%02d:%02d:%02d", gmt->tm_year +
    1900, gmt->tm_mon + 1, gmt->tm_mday, gmt->tm_hour, gmt->tm_min,
      gmt->tm_sec);
  return text;
}

/** Función para contar las líneas en el archivo de trabajo. */
uint64_t count_job_lines(const char* bin_name) {
  FILE* file;
  file = fopen(bin_name, "r");
  if (file == NULL) {
    fprintf(stderr, "Error opening file: %s\n", bin_name);
    return 0;
  }
  uint64_t num_lines = 0;
  while (!feof(file)) {
    char new_line = fgetc(file);
    if (new_line == '\n') {
      num_lines++;
    }
  }
  fclose(file);
  return num_lines;
}

/**
 * @brief Lee un archivo de trabajo (.txt) y almacena sus parámetros en un
 * arreglo de estructuras SimData.
 * 
 * @details Esta función abre un archivo de trabajo que contiene los parámetros
 * de simulación para múltiples simulaciones. Cada línea del archivo se procesa
 * para extraer los valores necesarios y se almacena en una estructura SimData.
 * 
 * @param job_name Nombre del archivo que contiene los parámetros de la
 * simulación.
 * @param dir Directorio donde se encuentra el archivo de trabajo.
 * @param lines Puntero donde se almacenará el número de líneas (simulaciones)
 * en el archivo.
 * @return Puntero a un arreglo de estructuras SimData que contiene los
 * parámetros de las simulaciones.
 */
SimData* read_job_file(const char* job_name, const char* dir,
  uint64_t* lines) {
  FILE* file;
  SimData* sim_params;
  char adress[512];

  /** Construir la ruta completa del archivo de trabajo. */
  snprintf(adress, sizeof(adress), "%s/%s", dir, job_name);
  /** Contar el número de líneas en el archivo de trabajo. */
  *lines = count_job_lines(adress);
  if (*lines <= 0) {
    fprintf(stderr, "Error reading job file\n");
    return 0;
  }
  /** Asignar memoria para almacenar los parámetros de cada simulación. */
  sim_params = malloc(*lines * sizeof(SimData));
  if (sim_params == NULL) {
    fprintf(stderr, "Error allocating memory for simulation parameters\n");
    return sim_params;
  }
  /** Abrir el archivo de trabajo en modo de lectura. */
  file = fopen(adress, "r");
  if (file == NULL) {
    fprintf(stderr, "Error opening job file: %s\n", adress);
    free(sim_params);
    return sim_params;
  }
  int i = 0;
  char temp_name[256];

  /** Leer los parámetros de cada línea del archivo de trabajo. */
  while (fscanf(file, "%s %lf %lf %lf %lf", temp_name, &sim_params[i].delta_t,
    &sim_params[i].alpha, &sim_params[i].h, &sim_params[i].epsilon) == 5) {
    sim_params[i].bin_name = malloc(strlen(temp_name) + 1);
    if (sim_params[i].bin_name == NULL) {
      fprintf(stderr,
        "Error allocating memory for bin_name at line: %d\n", i);
      break;
    }
    snprintf(sim_params[i].bin_name, sizeof(temp_name), "%s", temp_name);
    snprintf(adress, sizeof(adress), "%s/%s", dir, sim_params[i].bin_name);
    i++;
  }
  fclose(file);
  /** Retornar el arreglo de estructuras SimData. */
  return sim_params;
}

/**
 * @brief Crea un informe en formato .tsv con los resultados de la simulación.
 * 
 * @details Esta función genera un archivo de reporte con formato .tsv que
 * contiene información de cada simulación realizada. El informe incluye el
 * nombre del archivo binario, los parámetros de la simulación, el número de
 * estados y el tiempo simulado.
 * 
 * @param dir Directorio donde se guardará el archivo de reporte.
 * @param job_name Nombre del trabajo.
 * @param sim_params Puntero a un arreglo de estructuras que contiene los
 * parámetros de cada simulación.
 * @param num_states Puntero a un arreglo que contiene el número de estados
 * simulados para cada trabajo.
 * @param lines Número de líneas en el archivo de trabajo.
 */
void create_report(const char* dir, const char* job_name, SimData* sim_params,
  uint64_t* num_states, uint64_t lines) {
  char report_name[MAX_PATH_LENGTH];
  char base_job_name[512];
  char formatted_time[48];
  /** Copiar el nombre del archivo. */
  strncpy(base_job_name, job_name, sizeof(base_job_name) - 1);
  base_job_name[sizeof(base_job_name) - 1] = '\0';
  char* pos = strstr(base_job_name, ".txt");
  *pos = '\0';
  snprintf(report_name, sizeof(report_name), "%s/%s.tsv", dir, base_job_name);
  FILE* report_file = fopen(report_name, "w");
  if (report_file == NULL) {
    fprintf(stderr,
      "Error creating report file: %s\n", report_name);
    return;
  }
  for (uint64_t i = 0; i < lines; i++) {
    /** Capturar el tiempo transcurrido. */
    time_t time = num_states[i] * sim_params[i].delta_t;
    format_time(time, formatted_time, sizeof(formatted_time));
    /** Se escribe el archivo .tsv. */
    fprintf(report_file, "%s\t%lf\t%lf\t%lf\t%lg\t%lu\t%s\n",
      sim_params[i].bin_name, sim_params[i].delta_t, sim_params[i].alpha,
        sim_params[i].h, sim_params[i].epsilon, num_states[i], formatted_time);
  }
  fclose(report_file);
}

/**
 * @brief Escribe los datos de una matriz en un archivo binario.
 * 
 * @details Esta función guarda el estado actual de una simulación de calor en
 * un archivo binario. El archivo se nombra usando el nombre del trabajo y el
 * número de estados transcurridos.
 * 
 * @param data Matriz de datos que contiene los valores de la simulación.
 * @param rows Número de filas de la matriz.
 * @param cols Número de columnas de la matriz.
 * @param dir Directorio donde se guardará el archivo binario.
 * @param job_name Nombre del trabajo.
 * @param num_states Número de estados transcurridos en la simulación.
 */
void write_plate(double** data, uint64_t rows, uint64_t cols, const char* dir,
  const char* job_name, uint64_t num_states) {
  char file_name[MAX_PATH_LENGTH];
  char base_name[512];
  /** Copiar el nombre del archivo. */
  strncpy(base_name, job_name, sizeof(base_name) - 1);
  base_name[sizeof(base_name) - 1] = '\0';
  char* pos = strstr(base_name, ".bin");
  *pos = '\0';
  snprintf(file_name, sizeof(file_name), "%s/%s-%lu.bin", dir, base_name,
    num_states);
  FILE* output_file = fopen(file_name, "wb");
  if (output_file == NULL) {
    fprintf(stderr, "Error creating binary file: %s\n", file_name);
    return;
  }
  /** Escribir los datos en el archivo binario. */
  fwrite(&rows, sizeof(uint64_t), 1, output_file);
  fwrite(&cols, sizeof(uint64_t), 1, output_file);
  for (uint64_t i = 0; i < rows; i++) {
    fwrite(data[i], sizeof(double), cols, output_file);
  }
  fclose(output_file);
}
