// Copyright 2024 Josue Torres Sibaja <josue.torressibaja@ucr.ac.cr>

#include "heat_simulation.h"

/**
 * @brief Counts the number of non-empty lines in a text file.
 *
 * @param bin_name Pointer to the open file containing the job.
 * @return uint64_t Number of non-empty lines in the file.
 */
uint64_t count_job_lines(FILE* bin_name) {
  char line[256];
  uint64_t line_count = 0;
  while (fgets(line, sizeof(line), bin_name)) {
    // Checks if the line is not empty.
    if (line[0] != '\n' && strlen(line) > 0) {
      line_count++;
    }
  }
  fseek(bin_name, -2, SEEK_END);
  char empty;
  if ((empty = fgetc(bin_name)) == '\n') {
    line_count--;
  }
  rewind(bin_name);
  return line_count;
}

/**
 * @brief Reads the contents of the job file and returns its parameters.
 *
 * @param job_file Name of the job file to read.
 * @param struct_count Pointer to store the number of structures.
 * @return Structure containing the contents of the job file.
 */
SimData* read_job_file(const char* job_file, uint64_t* struct_count) {
  FILE* job = fopen(job_file, "r");
  if (!job) {
    perror("Error opening text file.");
    return NULL;
  }
  uint64_t lines_in_txt = count_job_lines(job);
  *struct_count = lines_in_txt;
  // Allocate memory for the structure array.
  SimData* simulation_parameters = (SimData*) calloc(lines_in_txt,
    sizeof(SimData));
  if (!simulation_parameters) {
    perror("Error allocating memory for simulation parameters.");
    fclose(job);
    return NULL;
  }
  char buffer[512];
  for (uint64_t line_number = 0; line_number < lines_in_txt; ++line_number) {
    if (fgets(buffer, sizeof(buffer), job)) {
      // Reads and assigns each argument in each structure.
      if (sscanf(buffer, "%255s %ld %lf %ld %lf",
        simulation_parameters[line_number].bin_name,
        &simulation_parameters[line_number].delta,
        &simulation_parameters[line_number].alpha,
        &simulation_parameters[line_number].h,
        &simulation_parameters[line_number].epsilon) != 5) {
        printf("Error analizing line %s.", buffer);
      }
    } else {
      printf("Error reading job line %lu.\n", line_number);
    }
  }
  fclose(job);
  return simulation_parameters;
}

/**
 * @brief Writes the simulation results to a report file.
 *
 * @param report_file Name of the report file (.tsv).
 * @param states Number of states until thermal equilibrium is reached.
 * @param time Time elapsed in the simulation.
 * @param params Structure containing the contents of the work file.
 * @param plate_filename Name of the binary file associated with the
 * simulation.
 */
void create_report(const char* report_file, uint64_t states, const char* time,
  SimData params, const char* plate_filename) {
  FILE* tsv_file = fopen(report_file, "a");
  if (!tsv_file) {
    perror("Error opening report file.");
    return;
  }
  // Write the report.
  fprintf(tsv_file, "%s\t%ld\t%g\t%ld\t%g\t%lu\t%s\n", plate_filename,
    params.delta, params.alpha, params.h, params.epsilon, states, time);
  fclose(tsv_file);
}

/**
 * @brief Writes the final state of the plate to a binary file.
 *
 * @param output_dir Directory where the binary file will be written.
 * @param data Data representing the plate.
 * @param rows Number of rows in the data.
 * @param cols Number of columns in the data.
 * @param states Number of states until equilibrium is reached.
 * @param plate_filename Name of the binary file associated with the
 * simulation.
 */
void write_plate(const char* output_dir, double** data, uint64_t rows,
  uint64_t cols, uint64_t states, const char* plate_filename) {
  // Get plate number.
  uint64_t plate_number = 0;
  sscanf(plate_filename, "plate%03lu.bin", &plate_number);
  // Create route to the .bin file.
  char path_to_bin[MAX_PATH_LENGTH];
  snprintf(path_to_bin, sizeof(path_to_bin), "%s/plate%03lu-%lu.bin",
    output_dir, plate_number, states);

  FILE* file = fopen(path_to_bin, "wb");
  if (!file) {
    perror("Error opening binary file for writing.");
    return;
  }
  // Write the number of rows.
  if (fwrite(&rows, sizeof(uint64_t), 1, file) != 1) {
    perror("Error writing binary rows.");
    fclose(file);
    return;
  }
  // Write the number of cols.
  if (fwrite(&cols, sizeof(uint64_t), 1, file) != 1) {
    perror("Error writing binary columns.");
    fclose(file);
    return;
  }
  // Write the temperatures.
  for (uint64_t i = 0; i < rows; i++) {
    if (fwrite(data[i], sizeof(double), cols, file) != cols) {
      perror("Error writing binary data.");
      fclose(file);
      return;
    }
  }
  fclose(file);
}

/**
 * @brief Formats elapsed time in seconds.
 *
 * @param seconds Time in seconds.
 * @param text String where the formatted time is stored.
 * @param capacity Capacity of the string.
 * @return Pointer to the formatted string.
 */
char* format_time(const time_t seconds, char* text, const size_t capacity) {
  const struct tm* gmt = gmtime(&seconds);  // NOLINT
  snprintf(text, capacity, "%04d/%02d/%02d\t%02d:%02d:%02d", gmt->tm_year - 70,
    gmt->tm_mon, gmt->tm_mday - 1, gmt->tm_hour, gmt->tm_min, gmt->tm_sec);
  return text;
}
