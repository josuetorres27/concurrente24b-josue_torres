// Copyright 2024 Josué Torres Sibaja <josue.torressibaja@ucr.ac.cr>

#ifndef HEAT_SIMULATION_H
#define HEAT_SIMULATION_H

/** Especificar el tamaño máximo permitido para las rutas de archivos. */
#define MAX_PATH_LENGTH 1024

/** Medir tiempo de ejecución. */
#define _POSIX_C_SOURCE 199309L

#include <assert.h>
#include <inttypes.h>
#include <math.h>
#include <pthread.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <time.h>
#include <unistd.h>

/**
 * @brief Estructura que almacena los parámetros de la simulación.
 *
 * @details Esta estructura guarda los parámetros necesarios para realizar la
 * simulación de calor en una lámina, incluyendo el nombre del archivo binario.
 */
typedef struct simulation_parameters {
  char bin_name[256];
  double alpha, epsilon;
  uint64_t delta_t, h;
} SimData;

/**
 * @brief Estructura que guarda los datos compartidos entre los hilos para la
 * simulación.
 *
 * @details Contiene la información de la lámina y sus propiedades, así como la
 * matriz de datos que representa las temperaturas de cada celda.
 */
typedef struct shared_thread_data {
  double** matrix;
  uint64_t cols, rows, delta_t, h, next_row;
  double alpha, epsilon;
  pthread_mutex_t matrix_mutex, work_mutex;
} SharedData;

/**
 * @brief Estructura que almacena los datos privados para cada hilo.
 *
 * @details Esta estructura define el rango de filas que un hilo debe procesar
 * durante la simulación.
 */
typedef struct private_thread_data {
  uint64_t start_row, end_row;
  SharedData* shared_data;
} ThreadData;

/** Declaración de funciones relacionadas con la simulación de calor. */
void configure_simulation(const char* plate_filename, SimData params,
  const char* filepath, const char* input_dir, uint64_t thread_count);
void simulate(uint64_t* states, uint64_t thread_count,
  SharedData* shared_data);
void* thread_sim(void* data);

/** Declaración de funciones auxiliares en utils.c. */
uint64_t count_job_lines(FILE* bin_name);
SimData* read_job_file(const char* job_file, uint64_t* struct_count);
void create_report(const char* report_file, uint64_t states, const char* time,
  SimData params, const char* plate_filename);
void write_plate(const char* output_dir, double** data, uint64_t rows,
  uint64_t cols, uint64_t states, const char* plate_filename);
char* format_time(const time_t seconds, char* text, const size_t capacity);

#endif  // HEAT_SIMULATION_H
