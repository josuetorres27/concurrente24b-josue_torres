// Copyright 2024 Josué Torres Sibaja <josue.torressibaja@ucr.ac.cr>

#include "plate.h"  // NOLINT

/** Función para contar las líneas en el archivo de trabajo. */
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

/**
 * @brief Crea un archivo de reporte con los resultados de la simulación.
 * 
 * @param job_file Nombre del archivo de trabajo.
 * @param plate_filename Nombre del archivo de la lámina.
 * @param delta_t Tiempo permitido entre un estado y otro.
 * @param alpha Coeficiente de difusión térmica.
 * @param h Alto y ancho de cada celda.
 * @param epsilon Mínimo cambio de temperatura significativo.
 * @param k Puntero donde se almacenará la cantidad de estados.
 * @param time_seconds Tiempo total en segundos que duró la simulación.
 * @param output_dir Directorio de salida donde se almacenará el reporte.
 * @return EXIT_SUCCESS si la operación es exitosa, o EXIT_FAILURE si ocurre
 * un error.
 */
int create_report(const char* job_file, const char* plate_filename,
  double delta_t, double alpha, double h, double epsilon, int k,
    time_t time_seconds, const char* output_dir) {
  /**
   * @brief Generar el nombre del archivo de reporte basado en job_file y
   * cambiar la extensión a .tsv.
   */
  char report_filename[MAX_PATH_LENGTH];
  /** Extraer el nombre base del archivo de trabajo sin extensión. */
  const char* base_name = strrchr(job_file, '/');
  if (!base_name) {
    /** Si no hay '/', el archivo está en el directorio actual. */
    base_name = job_file;
  } else {
    base_name++;  /** Saltar el '/'. */
  }

  /** Copiar el nombre base al buffer. */
  strncpy(report_filename, base_name, MAX_PATH_LENGTH - 1);
  report_filename[MAX_PATH_LENGTH - 1] = '\0';
  /** Modificar el nombre del archivo para que tenga la extensión .tsv. */
  char* dot_position = strrchr(report_filename, '.');
  if (dot_position != NULL) {
    strcpy(dot_position, ".tsv");  // NOLINT
  } else {
    strncat(report_filename, ".tsv", MAX_PATH_LENGTH - strlen(report_filename)
      - 1);
  }

  /** Construir la ruta completa con el directorio de salida. */
  char full_report_path[MAX_PATH_LENGTH];
  int written = snprintf(full_report_path, sizeof(full_report_path), "%s/%s",
    output_dir, report_filename);
  /** Verificar si la ruta al directorio de salida es demasiado larga. */
  if (written < 0 || written >= (int)sizeof(full_report_path)) {  // NOLINT
    fprintf(stderr, "Error: the full report path is too long\n");
    return EXIT_FAILURE;
  }

  /** Apertura del archivo de reporte. */
  FILE* report_file = fopen(full_report_path, "a");
  if (report_file == NULL) {
    perror("Error opening the report file");
    return EXIT_FAILURE;
  }
  char formatted_time[48]; /** Formatear el tiempo total de simulación. */
  format_time(time_seconds, formatted_time, sizeof(formatted_time));
  /** Escribir los datos en el archivo de reporte. */
  fprintf(report_file, "%s\t%lf\t%lf\t%lf\t%lf\t%d\t%s\n", plate_filename,
    delta_t, alpha, h, epsilon, k, formatted_time);
  fclose(report_file);
  return EXIT_SUCCESS;
}
