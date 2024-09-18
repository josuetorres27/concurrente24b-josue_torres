// Copyright 2024 Josué Torres Sibaja <josue.torressibaja@ucr.ac.cr>

#ifndef TAREAS_SERIAL_SRC_PLATE_H_
#define PLATE_H

#include <math.h>
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>  /** Para crear el directorio de salida si no existe. */
#include <sys/types.h>
#include <time.h>
#include <unistd.h>

/** Especificar el tamaño máximo permitido para las rutas de archivos. */
#define MAX_PATH_LENGTH 512

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

typedef struct SharedData {
  pthread_mutex_t write_mutex;
  int current_line;
} SharedData;

typedef struct SimulationData {
  char input_filepath[MAX_PATH_LENGTH];
  char plate_filename[MAX_PATH_LENGTH];
  const char* job_file;
  double delta_t, alpha, h, epsilon;
  int line_number;
  const char* output_dir;
  SharedData* shared_data;  /** Puntero a la estructura compartida. */
} SimulationData;

/** Declaración de funciones relacionadas con Plate. */
int read_dimensions(const char* filepath, Plate* plate);
int read_plate(const char* filepath, Plate* plate);
void simulate(Plate* plate, double delta_t, double alpha, double h,
  double epsilon, int* k, time_t* time_seconds);
int write_plate(const char* filepath, Plate* plate);

/** Declaración de funciones auxiliares en utils.c. */
char* format_time(const time_t seconds, char* text, const size_t capacity);
int create_report(const char* job_file, const char* plate_filename,
  double delta_t, double alpha, double h, double epsilon, int k,
    time_t time_seconds, const char* output_dir);

#endif  // TAREAS_SERIAL_SRC_PLATE_H_
