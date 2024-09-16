// Copyright 2024 Josué Torres Sibaja <josue.torressibaja@ucr.ac.cr>

#include "plate.h"

/**
 * @brief Lee las dimensiones de la matriz desde el archivo binario.
 * 
 * @details Esta función abre el archivo binario especificado por filepath, 
 * lee las dimensiones de la placa térmica desde los primeros 16 bytes del 
 * archivo y las almacena en la estructura Plate.
 * @param filepath Ruta del archivo binario que contiene las dimensiones de la 
 * matriz.
 * @param plate Puntero a la estructura Plate donde se almacenarán las 
 * dimensiones leídas.
 * @return EXIT_SUCCESS si la operación es exitosa, o EXIT_FAILURE si ocurre 
 * un error.
 */
int read_dimensions(const char* filepath, Plate* plate) {
  FILE* file = fopen(filepath, "rb");
  if (file == NULL) {
    perror("Error opening the plate file");
    return EXIT_FAILURE;
  }

  /** Leer las dimensiones (8 bytes para filas, 8 bytes para columnas). */
  if (fread(&(plate->rows), sizeof(long long int), 1, file) != 1 ||
    fread(&(plate->cols), sizeof(long long int), 1, file) != 1) {
    perror("Error reading plate dimensions");
    fclose(file);
    return EXIT_FAILURE;
  }
  fclose(file);
  return EXIT_SUCCESS;
}

/**
 * @brief Lee los datos de una placa térmica desde un archivo binario.
 * 
 * @details Esta función abre un archivo binario que contiene los datos de una 
 * placa térmica y lee los datos de la matriz en la estructura Plate 
 * proporcionada.
 * 
 * @param[in] filepath Ruta al archivo binario que contiene los datos.
 * @param[in] plate Puntero a la estructura Plate donde se almacenarán los 
 * datos leídos.
 * @return EXIT_SUCCESS si la operación es exitosa, o EXIT_FAILURE si ocurre 
 * un error.
 */
int read_plate(const char* filepath, Plate* plate) {
  FILE* file = fopen(filepath, "rb");
  if (file == NULL) {
    perror("Error opening the plate file");
    return EXIT_FAILURE;
  }

  /** Saltar los primeros 16 bytes que contienen las dimensiones. */
  fseek(file, 2 * sizeof(long long int), SEEK_SET);

  /**
   * @brief Asigna memoria para un arreglo de punteros, donde cada uno es una 
   * fila de la placa térmica.
   */
  plate->data = (double**) malloc(plate->rows * sizeof(double *));  // NOLINT

  /** Se asigna memoria para cada fila de la matriz. */
  for (long long int i = 0; i < plate->rows; i++) {
    plate->data[i] = (double*) malloc(plate->cols * sizeof(double));  // NOLINT

    /**
     * @brief Leer los datos de la placa térmica desde el archivo binario y 
     * almacenarlos en la matriz.
     */
    if (fread(plate->data[i], sizeof(double), plate->cols,
      file) != plate->cols) {
      perror("Error reading plate data");
      fclose(file);
      return EXIT_FAILURE;
    }
  }
  fclose(file);
  return EXIT_SUCCESS;
}

/**
 * @brief Simula la difusión térmica en una placa hasta alcanzar un estado de 
 * equilibrio.
 * 
 * @details Esta función realiza la simulación de la difusión térmica en la 
 * placa representada por la estructura Plate. La simulación se ejecuta 
 * iterativamente hasta que la diferencia máxima entre los valores de 
 * temperatura de la placa sea menor que un valor epsilon, lo que indica que 
 * se ha alcanzado el equilibrio térmico.
 * 
 * @param
 */
void* simulate(void* arg) {
  /** Inicialización de estructuras de datos. */
  ThreadData* data = (ThreadData*) arg;
  Plate* plate = data->shared_data->plate;
  double delta_t = data->shared_data->delta_t;
  double alpha = data->shared_data->alpha;
  double h = data->shared_data->h;
  double epsilon = data->shared_data->epsilon;
  int* k = data->shared_data->k;
  double max_delta = 0.0;

  double** next_plate = (double**) malloc((data->private_data.end_row -
    data->private_data.start_row) * sizeof(double*));  // NOLINT
  for (long long int i = 0; i < (data->private_data.end_row -
    data->private_data.start_row); i++) {
    next_plate[i] = (double*) malloc(plate->cols * sizeof(double));  // NOLINT
  }

  /** Algoritmo de simulación de calor. */
  do {
    for (long long int i = data->private_data.start_row; i <
      data->private_data.end_row; i++) {
      for (long long int j = 1; j < plate->cols - 1; j++) {
        /** Aplica la fórmula para calcular la nueva temperatura. */
        next_plate[i - data->private_data.start_row][j] = plate->data[i][j] +
          (((delta_t * alpha) / (h * h)) * (plate->data[i-1][j] +
            plate->data[i+1][j] + plate->data[i][j-1] + plate->data[i][j+1] -
              (4 * plate->data[i][j])));

        double delta = fabs(next_plate[i][j] - plate->data[i][j]);
        if (delta > max_delta) {
          max_delta = delta;
        }
      }
    }
    /** Proteger el acceso al contador global y tiempo con mutex. */
    pthread_mutex_lock(&data->shared_data->mutex);

    /** Copiar nuevas temperaturas a la matriz principal. */
    for (long long int i = data->private_data.start_row; i <
      data->private_data.end_row; i++) {
      for (long long int j = 1; j < plate->cols - 1; j++) {
        plate->data[i][j] = next_plate[i - data->private_data.start_row][j];
      }
    }
    (*data->shared_data->k)++;  /** Incrementar el contador de iteraciones. */
    *data->shared_data->time_seconds += delta_t;  /** Incrementar el tiempo. */

    pthread_mutex_unlock(&data->shared_data->mutex);

  } while (max_delta > epsilon); /** Condición de parada. */
  for (long long int i = 0; i < (data->private_data.end_row -
    data->private_data.start_row); i++) { /** Liberar memoria. */
    free(next_plate[i]);
  }
  free(next_plate);
  pthread_exit(NULL);
}

/**
 * @brief Escribe la matriz de temperaturas en un archivo binario.
 * 
 * @details Esta función guarda la matriz de temperaturas de la placa en un 
 * archivo binario especificado por filepath. Los datos se escriben en orden 
 * fila por fila.
 * @param filepath Nombre del archivo a crear.
 * @param plate Puntero a la estructura Plate que contiene la matriz de datos.
 * @return EXIT_SUCCESS si la operación es exitosa, o EXIT_FAILURE si ocurre 
 * un error.
 */
int write_plate(const char* filepath, Plate* plate) {
  FILE* file = fopen(filepath, "wb");
  if (file == NULL) {
    perror("Error opening the output file");
    return EXIT_FAILURE;
  }

  /** Escritura de las dimensiones de la matriz. */
  fwrite(&(plate->rows), sizeof(long long int), 1, file);
  fwrite(&(plate->cols), sizeof(long long int), 1, file);

  /** Escribe cada fila de la matriz de temperaturas en el archivo binario. */
  for (long long int i = 0; i < plate->rows; i++) {
    fwrite(plate->data[i], sizeof(double), plate->cols, file);
  }
  fclose(file);
  return EXIT_SUCCESS;
}
