#ifndef PLATE_H
#define PLATE_H

#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

/**
 * @brief Especificar el tamaño máximo permitido para las rutas de archivos.
 */
#define MAX_PATH_LENGTH 260

/**
 * @struct Plate
 * @brief Estructura para representar una placa térmica en la simulación.
 * 
 * @details Esta estructura almacena las dimensiones y los datos de la placa 
 * térmica utilizada en la simulación.
 */
typedef struct {
  long long int rows; /** Número de filas. */
  long long int cols; /** Número de columnas. */
  double** data; /** Puntero a la matriz de datos. */
} Plate;

/**
 * @brief Declaración de funciones relacionadas con Plate.
 */
int read_dimensions(const char* filepath, Plate* plate);
int read_plate(const char* filepath, Plate* plate);
void simulate(Plate* plate, double delta_t, double alpha, double h,
  double epsilon, int* k, time_t* time_seconds);
int write_plate(const char* filepath, Plate* plate);

/**
 * @brief Declaración de funciones auxiliares en utils.c.
 */
char* format_time(const time_t seconds, char* text, const size_t capacity);
int create_report(const char* job_file, const char* plate_filename,
  double delta_t, double alpha, double h, double epsilon, int k,
    time_t time_seconds);

#endif // PLATE_H
