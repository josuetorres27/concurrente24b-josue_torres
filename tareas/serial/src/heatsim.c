// Tarea 01
// Josué Torres Sibaja C37853

/**
 * @file heatsim.c
 * @brief
 * 
 * @details
 */

#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define MAX_PATH_LENGTH 260

/**
 * @brief Lee las dimensiones de la matriz desde el archivo binario.
 * 
 * @details
 * @param
 * @return
 */
void read_dimensions(const char *filepath, long long int *rows,
  long long int *cols) {
  FILE *file = fopen(filepath, "rb");
  if (file == NULL) {
    perror("Error opening the plate file");
    exit(EXIT_FAILURE);
  }

  /**
   * @brief Leer las dimensiones (8 bytes para filas, 8 bytes para columnas).
   */
  if (fread(rows, sizeof(long long int), 1, file) != 1 || fread(cols,
    sizeof(long long int), 1, file) != 1) {
    perror("Error reading plate dimensions");
    fclose(file);
    exit(EXIT_FAILURE);
  }

  fclose(file);

  printf("Filas: %lld\n", *rows);
  printf("Columnas: %lld\n", *cols);
}

/**
 * @brief Lee la matriz de temperaturas desde un archivo binario.
 * 
 * @details
 * @param
 * @return
 */
void read_plate(const char *filepath, long long int rows, long long int cols,
  double** plate) {
  FILE *file = fopen(filepath, "rb");
  if (file == NULL) {
    perror("Error opening the plate file");
    exit(EXIT_FAILURE);
  }

  /**
   * @brief Saltar los primeros 16 bytes que contienen las dimensiones.
   */
  fseek(file, 2 * sizeof(long long int), SEEK_SET);

  /**
   * @brief Leer los datos de la matriz.
   */
  for (long long int i = 0; i < rows; i++) {
    if (fread(plate[i], sizeof(double), cols, file) != cols) {
      perror("Error reading plate data");
      fclose(file);
      exit(EXIT_FAILURE);
    }

    // Imprimir cada valor leído para comprobar que todo está bien
    for (long long int j = 0; j < cols; j++) {
      printf("plate[%lld][%lld] = %f\n", i, j, plate[i][j]);
    }
  }

  fclose(file);
}

/**
 * @brief .
 * 
 * @details
 * @param
 * @return
 */
// Función principal
int main(int argc, char *argv[]) {
  if (argc < 2 || argc > 4) {
    fprintf(stderr, "Usage: %s <job file> [thread count] [file directory]\n",
      argv[0]);
    return EXIT_FAILURE;
  }

  const char *job_file = argv[1];
  int num_threads = (argc >= 3) ? atoi(argv[2]) : 1;

  /**
   * @brief Usar "." como directorio actual por defecto.
   */
  const char *dir_prefix = (argc == 4) ? argv[3] : ".";

  FILE *file = fopen(job_file, "r");
  if (file == NULL) {
    perror("Error opening the job file");
    return EXIT_FAILURE;
  }

  char plate_filename[MAX_PATH_LENGTH];
  double delta_t, alpha, h, epsilon;

  while (fscanf(file, "%s %lf %lf %lf %lf", plate_filename, &delta_t, &alpha,
    &h, &epsilon) == 5) {
    // Imprimir los valores leídos del archivo de trabajo
    printf("Reading file: %s, delta_t: %f, alpha: %f, h: %f, epsilon: %f\n",
      plate_filename, delta_t, alpha, h, epsilon);

    char filepath[MAX_PATH_LENGTH];
    int written = snprintf(filepath, sizeof(filepath), "%s/%s", dir_prefix,
      plate_filename);

    /**
     * @brief .
     */
    // Verificar si el resultado fue truncado
    if (written >= sizeof(filepath)) {
      fprintf(stderr, "Error: the file path is too long\n");
      fclose(file);
      return EXIT_FAILURE;
    }

    long long int rows, cols;

    /**
     * @brief Leer las dimensiones desde el archivo binario.
     */
    read_dimensions(filepath, &rows, &cols);

    /**
     * @brief Inicializar la matriz con las dimensiones leídas.
     */
    double** plate = (double**) malloc(rows * sizeof(double *));
    for (int i = 0; i < rows; i++) {
      plate[i] = (double*) malloc(cols * sizeof(double));
    }

    /**
     * @brief Leer la matriz inicial desde el archivo binario.
     */
    read_plate(filepath, rows, cols, plate);

    for (int i = 0; i < rows; i++) { /**< Liberar la memoria. */
      free(plate[i]);
    }
    free(plate);
  }

  fclose(file);
  return EXIT_SUCCESS;
}