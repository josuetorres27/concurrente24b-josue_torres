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

typedef struct SharedData {
  double** data;
  uint64_t cols;
  uint64_t rows;
  double delta_t, alpha, h, epsilon;
} SharedData;

typedef struct ThreadData {
  uint64_t start_row;
  uint64_t end_row;
  SharedData* shared_data;
} ThreadData;

typedef struct SimulationData {
  char* filename;
  double delta_t, alpha, h, epsilon;
} SimulationData;

/** Declaración de funciones relacionadas con Plate. */
SimulationData* read_job_file(const char* job_file, const char* dir,
  uint64_t* job_lines);
void thread_simulation(SimulationData* sim_params, const char* job_file,
  const char* dir, uint64_t job_lines, uint64_t num_threads);
//uint64_t simulate();
//void write_plate();
//void create_report();

/** Declaración de funciones auxiliares en utils.c. */
uint64_t count_job_lines(const char* job_file);
char* format_time(const time_t seconds, char* text, const size_t capacity);

#endif  // TAREAS_PTHREAD_SRC_PLATE_H_
