// Copyright 2024 Josué Torres Sibaja <josue.torressibaja@ucr.ac.cr>

#ifndef TAREAS_PTHREAD_SRC_PLATE_H_
#define PLATE_H

#include <ctype.h>
#include <math.h>
#include <pthread.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>  /** Para crear el directorio de salida si no existe. */
#include <sys/types.h>
#include <time.h>
#include <unistd.h>

/** Especificar el tamaño máximo permitido para las rutas de archivos. */
#define MAX_PATH_LENGTH 1024

/**
 * @struct Plate
 * @brief Estructura para representar una placa térmica en la simulación.
 * 
 * @details Esta estructura almacena las dimensiones y los datos de la placa
 * térmica utilizada en la simulación.
 */
typedef struct {
  int64_t rows; /** Número de filas. */
  int64_t cols; /** Número de columnas. */
  double** data; /** Puntero a la matriz de datos. */
} Plate;

typedef struct SharedData {
  pthread_mutex_t write_mutex;
  int current_turn; /** Controla cuál hilo debe escribir. */
  pthread_cond_t turn_cond; /** Condición para sincronizar la escritura. */
} SharedData;

typedef struct SimulationData {
  char input_filepath[MAX_PATH_LENGTH];
  char plate_filename[MAX_PATH_LENGTH];
  const char* job_file;
  double delta_t, alpha, h, epsilon;
  /** Índice del hilo para garantizar el orden de escritura. */
  int thread_index;
  const char* output_dir;
  SharedData* shared_data;  /** Puntero a la estructura compartida. */
} SimulationData;

/** Declaración de funciones relacionadas con Plate. */
int read_dimensions(const char* filepath, Plate* plate);
int read_plate(const char* filepath, Plate* plate);
void* process_simulation(void* arg);
void simulate(Plate* plate, double delta_t, double alpha, double h,
  double epsilon, int* k, time_t* time_seconds);
int write_plate(const char* filepath, Plate* plate);

/** Declaración de funciones auxiliares en utils.c. */
int count_job_lines(const char* job_file);
char* format_time(const time_t seconds, char* text, const size_t capacity);
int create_report(const char* job_file, const char* plate_filename,
  double delta_t, double alpha, double h, double epsilon, int k,
    time_t time_seconds, const char* output_dir);

#endif  // TAREAS_PTHREAD_SRC_PLATE_H_
