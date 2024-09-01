// Tarea_01
// Josué Torres Sibaja C37853

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <math.h>
#include <time.h>

// Estructura para representar una lámina
typedef struct {
  uint64_t rows;
  uint64_t cols;
  double* data;
} Plate;

// Función para inicializar una lámina
Plate create_plate(uint64_t rows, uint64_t cols, double initial_temperature) {
  Plate plate;
  plate.rows = rows;
  plate.cols = cols;
  plate.data = (double*) malloc(rows * cols * sizeof(double));
  if (plate.data == NULL) {
    perror("Error allocating memory for plate data");
    exit(EXIT_FAILURE);
  }
  for (uint64_t i = 0; i < rows * cols; i++) {
    plate.data[i] = initial_temperature;
  }
  return plate;
}

int main(void) {
  // Crear una lámina
  uint64_t rows = 10, cols = 10;
  double initial_temperature = 20.0;
  Plate plate = create_plate(rows, cols, initial_temperature);

  // Liberar memoria
  free(plate.data);

  return 0;
}