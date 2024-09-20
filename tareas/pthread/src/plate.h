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

/**
 * @struct SharedData
 * 
 * @brief Estructura que almacena datos compartidos entre los hilos.
 * 
 * @details Se utiliza para sincronizar el acceso a recursos compartidos.
 */
typedef struct SharedData {
  /**
   * Mutex para controlar el acceso a la escritura y evitar condiciones de
   * carrera.
   */
  pthread_mutex_t write_mutex;
  int current_turn; /** Controla cuál hilo debe escribir. */
  pthread_cond_t turn_cond; /** Condición para sincronizar la escritura. */
} SharedData;

/**
 * @struct SimulationData
 * 
 * @brief Estructura que contiene los datos necesarios para cada hilo de
 * simulación.
 * 
 * @details La información es específica de cada trabajo que se ejecuta en
 * paralelo.
 */
typedef struct SimulationData {
  /** Ruta del archivo de entrada que contiene los datos de la placa. */
  char input_filepath[MAX_PATH_LENGTH];
  /** Nombre del archivo que contiene la placa térmica. */
  char plate_filename[MAX_PATH_LENGTH];
  /** Archivo que contiene la descripción de los trabajos a procesar. */
  const char* job_file;
  /** Parámetros de simulación. */
  double delta_t, alpha, h, epsilon;
  /** Índice del hilo para garantizar el orden de escritura. */
  int thread_index;
  /** Directorio de salida donde se escribirán los archivos generados. */
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
