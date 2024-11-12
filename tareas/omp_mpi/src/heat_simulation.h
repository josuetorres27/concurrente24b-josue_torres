// Copyright 2024 Josue Torres Sibaja <josue.torressibaja@ucr.ac.cr>

#ifndef HEAT_SIMULATION_H
#define HEAT_SIMULATION_H

// Specify the maximum size allowed for file paths.
#define MAX_PATH_LENGTH 1024

#include <assert.h>
#include <inttypes.h>
#include <math.h>
#include <omp.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <time.h>
#include <unistd.h>

/**
 * @brief Structure that stores the simulation parameters.
 *
 * @details This structure stores the parameters needed to perform the heat
 * simulation on a plate, including the name of the binary file.
 */
typedef struct simulation_parameters {
  char bin_name[256];
  double alpha, epsilon;
  uint64_t delta, h;
} SimData;

/**
 * @brief Structure that stores the data shared between the threads for the
 * simulation.
 *
 * @details Contains the information about the plate and its properties, as
 * well as the data matrix that represents the temperatures of each cell.
 */
typedef struct shared_thread_data {
  double** matrix;
  uint64_t cols, rows, delta, h;
  double alpha, epsilon;
} SharedData;

// Declaration of functions related to heat simulation.
void configure_simulation(const char* plate_filename, SimData params,
  const char* filepath, const char* input_dir, uint64_t thread_count);
void simulate(uint64_t* states, SharedData* shared_data);

// Declaration of auxiliary functions in utils.c.
uint64_t count_job_lines(FILE* bin_name);
SimData* read_job_file(const char* job_file, uint64_t* struct_count);
void create_report(const char* report_file, uint64_t states, const char* time,
  SimData params, const char* plate_filename);
void write_plate(const char* output_dir, double** data, uint64_t rows,
  uint64_t cols, uint64_t states, const char* plate_filename);
char* format_time(const time_t seconds, char* text, const size_t capacity);

#endif  // HEAT_SIMULATION_H
