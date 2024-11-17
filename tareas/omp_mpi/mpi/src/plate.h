// Copyright 2024 Josue Torres Sibaja <josue.torressibaja@ucr.ac.cr>

#ifndef PLATE_H
#define PLATE_H

// Maximum allowed length for file paths.
#define MAX_PATH_LENGTH 1024

// For high-precision timing.
#define _POSIX_C_SOURCE 199309L

#include <mpi.h>
#include <math.h>
#include <time.h>
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <stdbool.h>
#include <inttypes.h>

// Structure to store simulation parameters for a specific plate.
typedef struct simulation_parameters {
  uint64_t delta;  ///< Time interval for the simulation.
  uint64_t h;      ///< Spatial distance between nodes in the plate.
  double alpha;    ///< Coefficient for heat transfer calculations.
  double epsilon;  ///< Convergence tolerance for calculations.
  char* bin_name;  ///< Name of the associated binary file.
} SimData;

// Structure to store shared data for simulation calculations.
typedef struct shared_thread_data {
  uint64_t rows;         ///< Number of rows in the simulation plate.
  uint64_t cols;         ///< Number of columns in the simulation plate.
  uint64_t delta;        ///< Time interval for the simulation.
  uint64_t h;            ///< Spatial distance between nodes in the plate.
  double alpha;          ///< Coefficient for heat transfer calculations.
  double epsilon;        ///< Convergence tolerance for calculations.
  double alpha_delta;    ///< Combined parameter for optimized calculations.
  double** matrix;       ///< Main plate matrix for the simulation.
  double** temp_matrix;  ///< Temporary matrix for intermediate calculations.
} SharedData;

// Functions declaration.
uint64_t simulate(SharedData* shared_data, int rank, int size);
void read_plate(const char* dir, SimData* sim_params, uint64_t lines,
  const char* job_name, int rank, int size);
double** create_matrix(uint64_t rows, uint64_t cols);
void free_matrix(double** matrix, uint64_t rows);
SimData* read_job_file(const char* job_name, const char* dir, uint64_t* lines);
void create_report(const char* dir, const char* job_name, SimData* sim_params,
  uint64_t* sim_states, uint64_t lines);
void write_plate(double** matrix, uint64_t rows, uint64_t cols,
  const char* dir, const char* job_name, uint64_t sim_states);
char* format_time(const time_t seconds, char* text, const size_t capacity);
uint64_t count_job_lines(const char* bin_name);

#endif  // PLATE_H
