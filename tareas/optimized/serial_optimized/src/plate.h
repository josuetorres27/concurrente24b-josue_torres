// Copyright 2024 Josué Torres Sibaja <josue.torressibaja@ucr.ac.cr>

#ifndef PLATE_H
#define PLATE_H

/** Especificar el tamaño máximo permitido para las rutas de archivos. */
#define MAX_PATH_LENGTH 1024

#include <inttypes.h>
#include <math.h>
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
  double delta_t;
  double alpha;
  double h;
  double epsilon;
} SimData;

/** Declaración de funciones auxiliares en utils.c. */
uint64_t count_job_lines(FILE* bin_name);
SimData* read_job_file(const char* job_file, uint64_t* struct_count);
void create_report(const char* report_file, uint64_t states, const char* time,
  SimData params, const char* plate_filename);
void write_plate(const char* output_dir, double** data, uint64_t rows,
  uint64_t cols, uint64_t states, const char* plate_filename);
char* format_time(const time_t seconds, char* text, const size_t capacity);

/** Declaración de funciones relacionadas con la simulación de calor. */
void configure_simulation(const char* plate_filename, SimData params,
  const char* filepath, const char* input_dir, const char* output_dir);
void simulate(double** data, uint64_t rows, uint64_t cols, SimData params,
  uint64_t* states);

#endif  // PLATE_H
