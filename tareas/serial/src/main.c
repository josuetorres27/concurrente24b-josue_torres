#include "plate.h"

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
