// Copyright 2024 Josué Torres Sibaja <josue.torressibaja@ucr.ac.cr>

#ifndef TAREAS_PTHREAD_SRC_HEAT_SIMULATION_H
#define TAREAS_PTHREAD_SRC_HEAT_SIMULATION_H

/** Especificar el tamaño máximo permitido para las rutas de archivos. */
#define MAX_PATH_LENGTH 1024

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <math.h>
#include <pthread.h>
#include <stdbool.h>
#include <time.h>
#include <inttypes.h>
#include <unistd.h>

/**
 * @brief Estructura que almacena los parámetros de la simulación.
 * 
 * @details Esta estructura guarda los parámetros necesarios para realizar la
 * simulación de calor en una lámina, incluyendo el nombre del archivo binario.
 */
typedef struct simulation_parameters {
  char* bin_name;
  double delta_t, alpha, h, epsilon;
} SimData;

/**
 * @brief Estructura que guarda los datos compartidos entre los hilos para la
 * simulación.
 * 
 * @details Contiene la información de la lámina y sus propiedades, así como la
 * matriz de datos que representa las temperaturas de cada celda.
 */
typedef struct shared_thread_data {
  double** data;
  uint64_t cols, rows;
  double delta_t, alpha, h, epsilon;
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
void configure_simulation(const char* dir, SimData* sim_params, uint64_t lines,
  const char* job_name, uint64_t thread_count);
uint64_t simulate(SharedData* shared_data, uint64_t thread_count);
void* thread_sim(void* data);

/** Declaración de funciones de lectura y escritura en utils.c. */
SimData* read_job_file(const char* job_name, const char* dir, uint64_t* lines);
void create_report(const char* dir, const char* job_name, SimData* sim_params,
  uint64_t* sim_states, uint64_t lines);
void write_plate(double** data, uint64_t rows, uint64_t cols, const char* dir,
  const char* job_name, uint64_t num_states);
char* format_time(const time_t seconds, char* text, const size_t capacity);
uint64_t count_job_lines(const char* bin_name);

#endif  // TAREAS_PTHREAD_SRC_HEAT_SIMULATION_H
