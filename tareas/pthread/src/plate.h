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
 * @brief Estructura para almacenar los datos compartidos entre hilos, 
 * incluyendo el mutex.
 */
typedef struct {
  Plate* plate;  /* Puntero a la estructura que contiene la matriz de datos. */
  double delta_t;  /* Tiempo permitido entre un estado y otro. */
  double alpha;  /* Coeficiente de difusión térmica. */
  double h;  /* Alto y ancho de cada celda. */
  double epsilon;  /* Mínimo cambio de temperatura significativo. */
  int* k;  /* Puntero donde se almacenará la cantidad de estados. */
  time_t* time_seconds;  /* Puntero para el tiempo total en segundos. */
  pthread_mutex_t* mutex;  /** Mutex compartido para proteger el acceso. */
} SharedData;

/** Estructura para los datos privados de cada hilo. */
typedef struct {
  /** Cada hilo procesa un subconjunto de filas con principio y fin. */
  long long int start_row;
  long long int end_row;
  int thread_id;
} PrivateData;

/** Estructura para agrupar los datos compartidos y privados. */
typedef struct {
  SharedData* shared_data;
  PrivateData private_data;
} ThreadData;

/** Declaración de funciones relacionadas con Plate. */
int read_dimensions(const char* filepath, Plate* plate);
int read_plate(const char* filepath, Plate* plate);
void* simulate(void* arg);
int write_plate(const char* filepath, Plate* plate);

/** Declaración de funciones auxiliares en utils.c. */
char* format_time(const time_t seconds, char* text, const size_t capacity);
int create_report(const char* job_file, const char* plate_filename,
  double delta_t, double alpha, double h, double epsilon, int k,
    time_t time_seconds, const char* output_dir);

#endif  // TAREAS_SERIAL_SRC_PLATE_H_
