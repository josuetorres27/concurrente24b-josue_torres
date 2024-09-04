// Tarea 01
// Josué Torres Sibaja C37853

/**
 * @file main.c
 * @brief Programa para realizar simulaciones de propagación de calor en una 
 * lámina.
 * 
 * @details El programa lee las dimensiones y valores iniciales de temperatura 
 * de una lámina desde un archivo binario, realiza una simulación de 
 * propagación de calor para encontrar el momento de equilibro térmico, y 
 * genera archivos de reporte y salida con los resultados.
 */

#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

/**
 * @brief Especificar el tamaño máximo permitido para las rutas de archivos.
 */
#define MAX_PATH_LENGTH 260

/**
 * @struct Plate
 * @brief Estructura para representar una placa térmica en la simulación.
 * 
 * @details Esta estructura almacena las dimensiones y los datos de la placa 
 * térmica utilizada en la simulación.
 */
typedef struct {
  long long int rows; /** Número de filas */
  long long int cols; /** Número de columnas */
  double** data; /** Puntero a la matriz de datos */
} Plate;

/**
 * @brief Formatea el tiempo transcurrido en segundos.
 * 
 * @details El formato de salida es: YYYY/MM/DD hh:mm:ss.
 * 
 * @param seconds Tiempo en segundos.
 * @param text Cadena de caracteres donde se guarda el tiempo formateado.
 * @param capacity Capacidad de la cadena de caracteres.
 * @return Puntero a la cadena de caracteres formateada.
 */
char* format_time(const time_t seconds, char* text, const size_t capacity) {
  const struct tm* gmt = gmtime(&seconds);
  snprintf(text, capacity, "%04d/%02d/%02d\t%02d:%02d:%02d",
    gmt->tm_year - 70, gmt->tm_mon, gmt->tm_mday - 1, gmt->tm_hour,
      gmt->tm_min, gmt->tm_sec);
  return text;
}

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

  /**
   * @brief Leer las dimensiones (8 bytes para filas, 8 bytes para columnas).
   */
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

  /**
   * @brief Saltar los primeros 16 bytes que contienen las dimensiones.
   */
  fseek(file, 2 * sizeof(long long int), SEEK_SET);

  /**
   * @brief Asigna memoria para un arreglo de punteros, donde cada uno es una 
   * fila de la placa térmica.
   */
  plate->data = (double**) malloc(plate->rows * sizeof(double *));

  /**
   * @brief Se asigna memoria para cada fila de la matriz.
   */
  for (long long int i = 0; i < plate->rows; i++) {
    plate->data[i] = (double*) malloc(plate->cols * sizeof(double));

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
 * @param plate Puntero a la estructura que contiene la matriz de datos.
 * @param delta_t Tiempo permitido entre un estado y otro.
 * @param alpha Coeficiente de difusión térmica.
 * @param h Alto y ancho de cada celda.
 * @param epsilon Mínimo cambio de temperatura significativo.
 * @param k Puntero donde se almacenará la cantidad de estados.
 * @param time_seconds Puntero donde se almacenará el tiempo total en segundos.
 */
void simulate(Plate* plate, double delta_t, double alpha, double h,
  double epsilon, int* k, time_t* time_seconds) {

  /**
   * @brief Inicialización de estructuras de datos.
   */
  double max_delta;
  double** next_plate = (double**) malloc(plate->rows * sizeof(double *));
  for (long long int i = 0; i < plate->rows; i++) {
    next_plate[i] = (double*) malloc(plate->cols * sizeof(double));
  }

  *k = 0; /** Declaración de contadores. */
  *time_seconds = 0;

  /**
   * @brief Algoritmo de simulación de calor.
   */
  do {
    max_delta = 0.0;
    for (long long int i = 1; i < plate->rows - 1; i++) {
      for (long long int j = 1; j < plate->cols - 1; j++) {

        /**
         * @brief Aplica la fórmula para calcular la nueva temperatura de cada 
         * celda.
         */
        next_plate[i][j] = plate->data[i][j] + (((delta_t * alpha) / (h * h)) *
          (plate->data[i-1][j] + plate->data[i+1][j] + plate->data[i][j-1] +
            plate->data[i][j+1] - (4 * plate->data[i][j])));

        double delta = fabs(next_plate[i][j] - plate->data[i][j]);
        if (delta > max_delta) {
          max_delta = delta;
        }
      }
    }

    /**
     * @brief Copiar nuevas temperaturas a la matriz principal.
     */
    for (long long int i = 1; i < plate->rows - 1; i++) {
      for (long long int j = 1; j < plate->cols - 1; j++) {
        plate->data[i][j] = next_plate[i][j];
      }
    }

    (*k)++; /** Incrementar contadores. */
    *time_seconds += delta_t;

  } while (max_delta > epsilon); /** Condición de parada. */

  for (long long int i = 0; i < plate->rows; i++) { /** Liberar memoria. */
    free(next_plate[i]);
  }
  free(next_plate);
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

  /**
   * @brief Escritura de las dimensiones de la matriz.
   */
  fwrite(&(plate->rows), sizeof(long long int), 1, file);
  fwrite(&(plate->cols), sizeof(long long int), 1, file);

  /**
   * @brief Escribe cada fila de la matriz de temperaturas en el archivo 
   * binario.
   */
  for (long long int i = 0; i < plate->rows; i++) {
    fwrite(plate->data[i], sizeof(double), plate->cols, file);
  }
  fclose(file);
  return EXIT_SUCCESS;
}

/**
 * @brief Crea un archivo de reporte con los resultados de la simulación.
 * 
 * @param job_file Nombre del archivo de trabajo.
 * @param plate_filename Nombre del archivo de la lámina.
 * @param delta_t Tiempo permitido entre un estado y otro.
 * @param alpha Coeficiente de difusión térmica.
 * @param h Alto y ancho de cada celda.
 * @param epsilon Mínimo cambio de temperatura significativo.
 * @param k Puntero donde se almacenará la cantidad de estados.
 * @param time_seconds Tiempo total en segundos que duró la simulación.
 * @return EXIT_SUCCESS si la operación es exitosa, o EXIT_FAILURE si ocurre 
 * un error.
 */
int create_report(const char* job_file, const char* plate_filename,
  double delta_t, double alpha, double h, double epsilon, int k,
    time_t time_seconds) {

  /**
   * @brief Generar el nombre del archivo de reporte.
   */
  char report_filename[MAX_PATH_LENGTH];
  strncpy(report_filename, job_file, MAX_PATH_LENGTH - 1);
  report_filename[MAX_PATH_LENGTH - 1] = '\0';

  /**
   * @brief Modificar el nombre del archivo para que tenga la extensión .tsv. 
   * Dependiendo de si el archivo original tiene la extensión .txt o no, la 
   * extensión será reemplazada o añadida al nombre.
   */
  char *dot_position = strrchr(report_filename, '.');
  if (dot_position != NULL && strcmp(dot_position, ".txt") == 0) {
  strcpy(dot_position, ".tsv");
  } else {
  strncat(report_filename, ".tsv", MAX_PATH_LENGTH - strlen(report_filename) -
    1);
  }

  /**
   * @brief Apertura del archivo de reporte.
   */
  FILE* report_file = fopen(report_filename, "a");
  if (report_file == NULL) {
    perror("Error opening the report file");
    return EXIT_FAILURE;
  }

  char formatted_time[48]; /** Formatear el tiempo total de simulación. */
  format_time(time_seconds, formatted_time, sizeof(formatted_time));

  /**
   * @brief Escribir los datos en el archivo de reporte.
   */
  fprintf(report_file, "%s\t%lf\t%lf\t%lf\t%lf\t%d\t%s\n", plate_filename,
    delta_t, alpha, h, epsilon, k, formatted_time);

  fclose(report_file);
  return EXIT_SUCCESS;
}

/**
 * @brief Función principal del programa.
 * 
 * @details Lee el archivo de trabajo especificado, realiza la simulación de 
 * propagación de calor para cada lámina, y genera archivos de reporte y de 
 * salida con los resultados.
 * 
 * @param argc Número de argumentos de la línea de comandos.
 * @param argv Arreglo de argumentos de la línea de comandos.
 * @return Código de retorno del programa.
 */
int main(int argc, char *argv[]) {
  /**
   * @brief Verifica que los argumentos proporcionados en la línea de comandos 
   * sean correctos.
   */
  if (argc < 2 || argc > 4) {
    fprintf(stderr, "Usage: %s <job file> [thread count] [file directory]\n",
      argv[0]);
    return EXIT_FAILURE;
  }

  /**
   * @brief Configura las variables y rutas necesarias, como el número de 
   * hilos y el directorio de trabajo.
   */
  const char* job_file = argv[1];
  int num_threads = (argc >= 3) ? atoi(argv[2]) : 1;
  const char* dir_prefix = (argc == 4) ? argv[3] : ".";

  /**
   * @brief Abre el archivo de trabajo especificado por el usuario, que 
   * contiene las especificaciones para la simulación.
   */
  FILE* file = fopen(job_file, "r");
  if (file == NULL) {
    perror("Error opening the job file");
    return EXIT_FAILURE;
  }

  char filepath[MAX_PATH_LENGTH];
  char plate_filename[MAX_PATH_LENGTH];
  double delta_t, alpha, h, epsilon;

  /**
   * @brief Bucle de lectura de configuración del archivo de trabajo.
   */
  while (fscanf(file, "%s %lf %lf %lf %lf", plate_filename, &delta_t, &alpha,
    &h, &epsilon) == 5) {

    /**
     * @brief Verificar el tamaño de la ruta.
     */
    int written = snprintf(filepath, sizeof(filepath), "%s/%s", dir_prefix,
      plate_filename);

    if (written < 0 || written >= sizeof(filepath)) {
      fprintf(stderr, "Error: the file path is too long\n");
      fclose(file);
      return EXIT_FAILURE;
    }

    /**
     * @brief Leer las dimensiones y datos de la placa.
     */
    Plate plate;
    if (read_dimensions(filepath, &plate) != EXIT_SUCCESS) {
      fclose(file);
      return EXIT_FAILURE;
    }

    if (read_plate(filepath, &plate) != EXIT_SUCCESS) {
      fclose(file);
      return EXIT_FAILURE;
    }

  /**
   * @brief Realiza una simulación para cada lámina especificada en el archivo 
   * de trabajo.
   * 
   * @details Para cada simulación realiza las siguientes acciones:
   * - Lee las dimensiones y la matriz de temperaturas iniciales desde un 
   * archivo binario.
   * - Realiza la simulación de propagación de calor hasta que se alcance el 
   * equilibrio térmico.
   * - Genera un archivo de reporte con los resultados de la simulación.
   * - Guarda el estado final de la matriz en un archivo binario.
   */
    int k;
    time_t time_seconds;
    simulate(&plate, delta_t, alpha, h, epsilon, &k, &time_seconds);

    /**
     * @brief Genera el reporte de la simulación.
     */
    create_report(job_file, plate_filename, delta_t, alpha, h, epsilon, k,
      time_seconds);

    /**
     * @brief Construcción y configuración del nombre del archivo de salida.
     */
    char output_filename[MAX_PATH_LENGTH];
    snprintf(output_filename, sizeof(output_filename), "plate%03d-%d.bin",
      atoi(&plate_filename[5]), k);
    write_plate(output_filename, &plate);

    /**
     * @brief Libera la memoria dinámica que se ha utilizado durante la 
     * ejecución.
     */
    for (long long int i = 0; i < plate.rows; i++) {
      free(plate.data[i]);
    }
    free(plate.data);
  }
  fclose(file);
  return EXIT_SUCCESS;
}