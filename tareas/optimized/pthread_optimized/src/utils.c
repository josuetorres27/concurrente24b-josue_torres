// Copyright 2024 Josué Torres Sibaja <josue.torressibaja@ucr.ac.cr>

#include "heat_simulation.h"

/**
 * @brief Cuenta el número de líneas no vacías en un archivo de texto.
 * 
 * @param bin_name Puntero al archivo abierto que contiene el trabajo.
 * @return uint64_t Número de líneas no vacías en el archivo.
 */
uint64_t count_job_lines(FILE* bin_name) {
  char line[256];
  uint64_t line_count = 0;
  while (fgets(line, sizeof(line), bin_name)) {
    /** Verifica si la línea no está vacía. */
    if (line[0] != '\n' && strlen(line) > 0) {
      line_count++;
    }
  }
  fseek(bin_name, -2, SEEK_END);
  char empty;
  if ((empty = fgetc(bin_name)) == '\n') {
    line_count--;
  }
  rewind(bin_name);
  return line_count;
}

/**
 * @brief Lee el contenido del archivo de trabajo y retorna sus parámetros.
 * 
 * @param job_file Nombre del archivo de trabajo a leer.
 * @param struct_count Puntero para almacenar el número de estructuras.
 * @return Estructura que contiene el contenido del archivo de trabajo.
 */
SimData* read_job_file(const char* job_file, uint64_t* struct_count) {
  FILE* job = fopen(job_file, "r");
  if (!job) {
    perror("Error opening text file.");
    return NULL;
  }
  uint64_t lines_in_txt = count_job_lines(job);
  *struct_count = lines_in_txt;
  // Asignar memoria para el arreglo de estructuras
  SimData* simulation_parameters = (SimData*) calloc(lines_in_txt,
    sizeof(SimData));
  if (!simulation_parameters) {
    perror("Error allocating memory for simulation parameters.");
    fclose(job);
    return NULL;
  }
  char buffer[512];
  for (uint64_t line_number = 0; line_number < lines_in_txt; ++line_number) {
    if (fgets(buffer, sizeof(buffer), job)) {
      /** Lee y asigna cada argumento en cada estructura. */
      if (sscanf(buffer, "%255s %ld %lf %ld %lf",
        simulation_parameters[line_number].bin_name,
        &simulation_parameters[line_number].delta_t,
        &simulation_parameters[line_number].alpha,
        &simulation_parameters[line_number].h,
        &simulation_parameters[line_number].epsilon) != 5) {
        printf("Error analizing line %s.", buffer);
      }
    } else {
      printf("Error reading job line %lu.\n", line_number);
    }
  }
  fclose(job);
  return simulation_parameters;
}

/**
 * @brief Escribe los resultados de la simulación en un archivo de reporte.
 * 
 * @param report_file Nombre del archivo de reporte (.tsv).
 * @param states Número de estados hasta alcanzar el equilibrio térmico.
 * @param time Tiempo transcurrido en la simulación.
 * @param params Estructura que contiene el contenido del archivo de trabajo.
 * @param plate_filename Nombre del archivo binario asociado con la simulación.
 */
void create_report(const char* report_file, uint64_t states, const char* time,
  SimData params, const char* plate_filename) {
  FILE* tsv_file = fopen(report_file, "a");
  if (!tsv_file) {
    perror("Error opening report file.");
    return;
  }
  /** Escribir el reporte. */
  fprintf(tsv_file, "%s\t%ld\t%g\t%ld\t%g\t%lu\t%s\n", plate_filename,
    params.delta_t, params.alpha, params.h, params.epsilon, states, time);
  fclose(tsv_file);
}

/**
 * @brief Escribe el estado final de la placa en un archivo binario.
 * 
 * @param output_dir Directorio donde se escribirá el archivo binario.
 * @param data Datos que representan la placa.
 * @param rows Número de filas en los datos.
 * @param cols Número de columnas en los datos.
 * @param states Número de estados hasta alcanzar el equilibrio.
 * @param plate_filename Nombre del archivo binario asociado con la simulación.
 */
void write_plate(const char* output_dir, double** data, uint64_t rows,
  uint64_t cols, uint64_t states, const char* plate_filename) {
  /** Obtener el número de lámina. */
  uint64_t plate_number = 0;
  sscanf(plate_filename, "plate%03lu.bin", &plate_number);
  /** Crear la ruta al archivo .bin. */
  char path_to_bin[MAX_PATH_LENGTH];
  snprintf(path_to_bin, sizeof(path_to_bin), "%s/plate%03lu-%lu.bin",
    output_dir, plate_number, states);

  FILE* file = fopen(path_to_bin, "wb");
  if (!file) {
    perror("Error opening binary file for writing.");
    return;
  }
  /** Escribir el número de filas. */
  if (fwrite(&rows, sizeof(uint64_t), 1, file) != 1) {
    perror("Error writing binary rows.");
    fclose(file);
    return;
  }
  /** Escribir el número de columnas. */
  if (fwrite(&cols, sizeof(uint64_t), 1, file) != 1) {
    perror("Error writing binary columns.");
    fclose(file);
    return;
  }
  /** Escribir las temperaturas. */
  for (uint64_t i = 0; i < rows; i++) {
    if (fwrite(data[i], sizeof(double), cols, file) != cols) {
      perror("Error writing binary data.");
      fclose(file);
      return;
    }
  }
  fclose(file);
}

/**
 * @brief Formatea el tiempo transcurrido en segundos.
 * 
 * @param seconds Tiempo en segundos.
 * @param text Cadena de caracteres donde se guarda el tiempo formateado.
 * @param capacity Capacidad de la cadena de caracteres.
 * @return Puntero a la cadena de caracteres formateada.
 */
char* format_time(const time_t seconds, char* text, const size_t capacity) {
  const struct tm* gmt = gmtime(&seconds);  // NOLINT
  snprintf(text, capacity, "%04d/%02d/%02d\t%02d:%02d:%02d", gmt->tm_year - 70,
    gmt->tm_mon, gmt->tm_mday - 1, gmt->tm_hour, gmt->tm_min, gmt->tm_sec);
  return text;
}
