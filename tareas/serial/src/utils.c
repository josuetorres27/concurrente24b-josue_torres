#include "plate.h"

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
  const struct tm* gmt = gmtime(&seconds);
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
 * @return EXIT_SUCCESS si la operación es exitosa, o EXIT_FAILURE si ocurre 
 * un error.
 */
int create_report(const char* job_file, const char* plate_filename,
  double delta_t, double alpha, double h, double epsilon, int k,
    time_t time_seconds) {
  /**
   * @brief Generar el nombre del archivo de reporte.
   */
  char report_filename[MAX_PATH_LENGTH];
  strncpy(report_filename, job_file, MAX_PATH_LENGTH - 1);
  report_filename[MAX_PATH_LENGTH - 1] = '\0';
  /**
   * @brief Modificar el nombre del archivo para que tenga la extensión .tsv. 
   * Dependiendo de si el archivo original tiene la extensión .txt o no, la 
   * extensión será reemplazada o añadida al nombre.
   */
  char *dot_position = strrchr(report_filename, '.');
  if (dot_position != NULL && strcmp(dot_position, ".txt") == 0) {
    strcpy(dot_position, ".tsv");
  } else {
    strncat(report_filename, ".tsv", MAX_PATH_LENGTH - strlen(report_filename) 
      - 1);
  }
  /**
   * @brief Apertura del archivo de reporte.
   */
  FILE* report_file = fopen(report_filename, "a");
  if (report_file == NULL) {
    perror("Error opening the report file");
    return EXIT_FAILURE;
  }
  char formatted_time[48]; /** Formatear el tiempo total de simulación. */
  format_time(time_seconds, formatted_time, sizeof(formatted_time));
  /**
   * @brief Escribir los datos en el archivo de reporte.
   */
  fprintf(report_file, "%s\t%lf\t%lf\t%lf\t%lf\t%d\t%s\n", plate_filename,
    delta_t, alpha, h, epsilon, k, formatted_time);
  fclose(report_file);
  return EXIT_SUCCESS;
}
