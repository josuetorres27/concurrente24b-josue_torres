// Copyright 2024 Josue Torres Sibaja <josue.torressibaja@ucr.ac.cr>

#include "plate.h"  // NOLINT

/**
 * @brief Reads a job file containing simulation parameters and stores them in
 * an array of SimData structs.
 *
 * @param job_name Name of the job file to read.
 * @param dir Directory where the job file is located.
 * @param lines Pointer to a variable where the number of lines in the job file
 * will be stored.
 * @return A dynamically allocated array of SimData structs containing the
 * simulation parameters. Returns NULL if an error occurs.
 */
SimData* read_job_file(const char* job_name, const char* dir,
  uint64_t* lines) {
  FILE *job_file;
  SimData *sim_params;
  char file_path[512];

  // Construct the full path to the job file.
  snprintf(file_path, sizeof(file_path), "%s/%s", dir, job_name);

  // Count the number of lines in the job file.
  *lines = count_job_lines(file_path);
  if (*lines <= 0) {
    fprintf(stderr, "The file contains 0 lines or could not be read.\n");
    return NULL;
  }

  // Allocate memory for the SimData array.
  sim_params = malloc(*lines * sizeof(SimData));
  if (sim_params == NULL) {
    fprintf(stderr, "Memory allocation failed.\n");
    return NULL;
  }

  // Open the job file for reading.
  job_file = fopen(file_path, "r");
  if (job_file == NULL) {
    fprintf(stderr, "Failed to open job file: %s.\n", file_path);
    free(sim_params);
    return NULL;
  }

  // Read each line from the job file and populate the SimData array.
  int i = 0;
  char temp_plate[256];
  while (fscanf(job_file, "%s %ld %lf %ld %lf", temp_plate,
    &sim_params[i].delta, &sim_params[i].alpha, &sim_params[i].h,
      &sim_params[i].epsilon) == 5) {
    sim_params[i].bin_name = malloc(strlen(temp_plate) + 1);
    if (sim_params[i].bin_name == NULL) {
      fprintf(stderr, "Memory allocation failed for bin_name.\n");
      break;
    }
    snprintf(sim_params[i].bin_name, sizeof(temp_plate), "%s", temp_plate);
    snprintf(file_path, sizeof(file_path), "%s/%s", dir,
      sim_params[i].bin_name);
    i++;
  }

  fclose(job_file);
  return sim_params;
}

/**
 * @brief Creates a report file in TSV format based on simulation data.
 *
 * @param dir Directory where the report file will be saved.
 * @param job_name Name of the original job file.
 * @param sim_params Array of SimData structs containing simulation parameters.
 * @param sim_states Array of simulation states for each SimData entry.
 * @param lines Number of lines (entries) in the arrays.
 */
void create_report(const char* dir, const char* job_name, SimData* sim_params,
  uint64_t* sim_states, uint64_t lines) {
  char report_name[1024];
  char root_name[512];
  char formatted_time[48];

  // Remove the ".txt" extension from the job name.
  strncpy(root_name, job_name, sizeof(root_name) - 1);
  root_name[sizeof(root_name) - 1] = '\0';
  char* pos = strstr(root_name, ".txt");
  *pos = '\0';

  // Construct the report file name with ".tsv" extension.
  snprintf(report_name, sizeof(report_name), "%s/%s.tsv", dir, root_name);

  FILE* report_file = fopen(report_name, "w");
  if (report_file == NULL) {
    fprintf(stderr, "Failed to create report file: %s.\n", report_name);
    return;
  }

  for (uint64_t i = 0; i < lines; i++) {
    // Calculate elapsed time.
    time_t elapsed_time = sim_states[i] * sim_params[i].delta;

    // Format elapsed time.
    format_time(elapsed_time, formatted_time, sizeof(formatted_time));

    // Write the simulation data to the report file.
    fprintf(report_file, "%s\t%ld\t%lf\t%ld\t%lg\t%lu\t%s\n",
      sim_params[i].bin_name, sim_params[i].delta, sim_params[i].alpha,
        sim_params[i].h, sim_params[i].epsilon, sim_states[i], formatted_time);
  }

  fclose(report_file);
}

/**
 * @brief Writes a matrix to a binary file.
 *
 * @param matrix Array of double values representing the plate simulation data.
 * @param rows Number of rows in the matrix.
 * @param cols Number of columns in the matrix.
 * @param dir Directory where the binary file will be saved.
 * @param job_name Base name of the job file.
 * @param sim_states Simulation state identifier used in the output file name.
 */
void write_plate(double** matrix, uint64_t rows, uint64_t cols,
  const char* dir, const char* job_name, uint64_t sim_states) {
  char file_name[1024];
  char root_name[512];

  // Remove the ".bin" extension from the job name.
  strncpy(root_name, job_name, sizeof(root_name) - 1);
  root_name[sizeof(root_name) - 1] = '\0';
  char* pos = strstr(root_name, ".bin");
  *pos = '\0';

  // Construct the binary file name.
  snprintf(file_name, sizeof(file_name), "%s/%s-%lu.bin", dir, root_name,
    sim_states);

  FILE* bin_file = fopen(file_name, "wb");
  if (bin_file == NULL) {
    fprintf(stderr, "Failed to create binary file: %s.\n", file_name);
    return;
  }

  // Write the matrix dimensions and data to the binary file.
  fwrite(&rows, sizeof(uint64_t), 1, bin_file);
  fwrite(&cols, sizeof(uint64_t), 1, bin_file);
  for (uint64_t i = 0; i < rows; i++) {
    fwrite(matrix[i], sizeof(double), cols, bin_file);
  }

  fclose(bin_file);
}

/**
 * @brief Formats elapsed time in a specific format (YYYY/MM/DD HH:MM:SS).
 *
 * @param seconds Elapsed time in seconds.
 * @param text Output buffer for the formatted time.
 * @param capacity Capacity of the output buffer.
 * @return Pointer to the formatted time string.
 */
char* format_time(const time_t seconds, char* text, const size_t capacity) {
  const struct tm* gmt = gmtime(&seconds);  // NOLINT
  snprintf(text, capacity, "%04d/%02d/%02d\t%02d:%02d:%02d", gmt->tm_year - 70,
    gmt->tm_mon, gmt->tm_mday - 1, gmt->tm_hour, gmt->tm_min, gmt->tm_sec);
  return text;
}

/**
 * @brief Counts the number of lines in a text file.
 *
 * @param bin_name Name of the file to be read.
 * @return Number of lines in the file. Returns 0 if the file cannot be read.
 */
uint64_t count_job_lines(const char* bin_name) {
  FILE *file = fopen(bin_name, "r");
  if (file == NULL) {
    fprintf(stderr, "Failed to open file: %s.\n", bin_name);
    return 0;
  }

  uint64_t line_count = 0;
  while (!feof(file)) {
    char new_line = fgetc(file);
    if (new_line == '\n') {
      line_count++;
    }
  }

  fclose(file);
  return line_count;
}
