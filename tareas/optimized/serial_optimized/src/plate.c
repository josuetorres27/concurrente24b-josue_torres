// Copyright 2024 Josué Torres Sibaja <josue.torressibaja@ucr.ac.cr>

#include "plate.h"

/**
 * @brief Lee el archivo binario de la placa, crea los datos y llama a las
 * funciones para ejecutar la simulación y escribir los archivos de salida.
 *
 * @param plate_filename Nombre del archivo binario que contiene los datos.
 * @param params Estructura que contiene los parámetros de la simulación.
 * @param filepath Ruta donde se guardarán el reporte y los archivos de salida.
 * @param input_dir Directorio donde se encuentra el archivo binario.
 * @param output_dir Directorio donde se guardará el archivo binario de salida.
 */
void configure_simulation(const char* plate_filename, SimData params,
  const char* filepath, const char* input_dir, const char* output_dir) {
  /** Crear la ruta hacia el archivo binario. */
  char bin_path[257];
  snprintf(bin_path, sizeof(bin_path), "%s/%s", input_dir, plate_filename);
  FILE* bin_file;
  uint64_t rows, cols;
  bin_file = fopen(bin_path, "rb");
  if (!bin_file) {
    fprintf(stderr, "Error opening binary file.\n");
    return;
  }

  /** Leer los valores de la lámina. */
  if (fread(&rows, sizeof(uint64_t), 1, bin_file) != 1) {
    fprintf(stderr, "Error reading number of rows.\n");
    fclose(bin_file);
    return;
  }
  if (fread(&cols, sizeof(uint64_t), 1, bin_file) != 1) {
    fprintf(stderr, "Error reading number of columns.\n");
    fclose(bin_file);
    return;
  }

  double** data = (double**) malloc(rows * sizeof(double*));
  if (!data) {
    fprintf(stderr, "Error allocating memory for plate rows.\n");
    fclose(bin_file);
    return;
  }

  for (uint64_t i = 0; i < rows; i++) {
    data[i] = (double*) malloc(cols * sizeof(double));
    if (!data[i]) {
      fprintf(stderr, "Error allocating memory for plate columns.\n");
      for (uint64_t j = 0; j < i; j++) {
        free(data[j]);
      }
      free(data);
      fclose(bin_file);
      return;
    }
  }

  for (uint64_t i = 0; i < rows; i++) {
    for (uint64_t j = 0; j < cols; j++) {
      if (fread(&data[i][j], sizeof(double), 1, bin_file) != 1) {
        fprintf(stderr, "Error reading plate data at row %" PRIu64 ", \
          col %" PRIu64 ".\n", i, j);
        for (uint64_t k = 0; k < rows; k++) {
          free(data[k]);
        }
        free(data);
        fclose(bin_file);
        return;
      }
    }
  }
  fclose(bin_file);

  /** Ejecutar la simulación. */
  uint64_t states = 0;
  simulate(data, rows, cols, params, &states);

  /** Tomar el tiempo transcurrido. */
  const time_t secs = states * params.delta_t;
  char time[49];
  format_time(secs, time, sizeof(time));

  /** Escribir la nueva placa. */
  write_plate(output_dir, data, rows, cols, states, plate_filename);
  for (uint64_t i = 0; i < rows; i++) {
    free(data[i]);
  }
  free(data);

  /** Escribir el reporte. */
  create_report(filepath, states, time, params, plate_filename);
}

/**
 * @brief Ejecuta la simulación térmica en cada lámina hasta que se alcance el
 * equilibrio térmico.
 *
 * @param data Datos que representan el estado inicial de la lámina.
 * @param rows Número de filas de la matriz.
 * @param cols Número de columnas de la matriz.
 * @param params Estructura que contiene los parámetros de la simulación.
 * @param states Puntero para almacenar el número de iteraciones necesarias
 * para alcanzar el equilibrio.
 */
void simulate(double** data, uint64_t rows, uint64_t cols, SimData params,
  uint64_t* states) {
  double** data_copy = (double**) malloc(rows * sizeof(double*));
  if (!data_copy) {
    fprintf(stderr, "Error allocating memory for plate copy rows.\n");
    return;
  }

  for (uint64_t i = 0; i < rows; i++) {
    data_copy[i] = (double*) malloc(cols * sizeof(double));
    if (!data_copy[i]) {
      fprintf(stderr, "Error allocating memory for data copy.\n");
      for (uint64_t j = 0; j < i; j++) {
        free(data_copy[j]);
      }
      free(data_copy);
      return;
    }
  }

  uint64_t state = 0;
  double max_epsilon = params.epsilon + 1;
  /** Continuar hasta alcanzar el equilibrio térmico. */
  while (max_epsilon > params.epsilon) {
    max_epsilon = 0.0;
    state++;
    /** Actualizar la temperatura de cada celda con la fórmula. */
    for (uint64_t i = 1; i < rows - 1; i++) {
      for (uint64_t j = 1; j < cols - 1; j++) {
        double cell = data[i][j];
        double cells_around = data[i - 1][j] + data[i][j + 1] + data[i + 1][j]
          + data[i][j - 1];
        /** Aplicar la fórmula. */
        data_copy[i][j] = cell + (params.delta_t * params.alpha / (params.h *
          params.h)) * (cells_around - 4 * cell);
        /** Calcular la diferencia máxima. */
        double difference = fabs(data_copy[i][j] - cell);
        if (difference > max_epsilon) {
          max_epsilon = difference;
        }
      }
    }
    /** Copiar cambios en la matriz original. */
    for (uint64_t i = 1; i < rows - 1; i++) {
      for (uint64_t j = 1; j < cols - 1; j++) {
        data[i][j] = data_copy[i][j];
      }
    }
  }
  *states = state;
  for (uint64_t i = 0; i < rows; i++) {
    free(data_copy[i]);
  }
  free(data_copy);
}
