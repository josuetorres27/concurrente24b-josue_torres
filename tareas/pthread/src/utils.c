// Copyright 2024 Josué Torres Sibaja <josue.torressibaja@ucr.ac.cr>

#include "plate.h"  // NOLINT

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
