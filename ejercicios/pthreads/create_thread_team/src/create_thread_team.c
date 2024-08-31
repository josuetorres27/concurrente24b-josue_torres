// Josué Torres Sibaja / C37853 / 2024
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>

// Estructura para los datos privados de cada hilo
typedef struct {
  size_t thread_number; // Número de hilo
  size_t thread_count; // Cantidad de hilos en el equipo
  void* shared_data; // Puntero a los datos compartidos
  pthread_t thread_id; // Identificador del hilo
} private_data;

int join_threads(size_t count, private_data* private_data_arr);

// Modificación de create_threads para usar la estructura
private_data* create_threads(size_t count, void* (*routine)(void*),
  void* data) {
  // Crea e inicializa un arreglo de registros de datos privados
  private_data* private_data_arr = (private_data*)
    calloc(count, sizeof(private_data));
  if (private_data_arr) {
    for (size_t index = 0; index < count; index++) {
      // Inicializar los datos privados del hilo
      private_data_arr[index].thread_number = index;
      private_data_arr[index].thread_count = count;
      private_data_arr[index].shared_data = data;

      // Envía la dirección del registro privado a cada hilo
      int error = pthread_create(&private_data_arr[index].thread_id, NULL,
        routine, &private_data_arr[index]);

      if (error != EXIT_SUCCESS) {
        fprintf(stderr, "Error: could not create thread %zu\n", index);
        join_threads(index, private_data_arr); // Limpieza en caso de error
        return NULL;
      }
    }
  }
  return private_data_arr;
}

// Recibe la cantidad de hilos y la dirección del arreglo de registros privados
int join_threads(size_t count, private_data* private_data_arr) {
  int error_count = 0; // Cuenta cuántos intentos de unir hilos fallan
  // Espera por cada hilo
  for (size_t index = 0; index < count; index++) {
    const int error = pthread_join(private_data_arr[index].thread_id, NULL);
    if (error) {
      fprintf(stderr, "Error: could not join thread %zu\n", index);
      error_count++;
    }
  }
  // Libera el arreglo de datos privados
  free(private_data_arr);
  return error_count;
}

// Ejemplo de una rutina de hilo
void* routine(void* arg) {
  private_data* data = (private_data*) arg;
  printf("I am thread %zu of %zu\n", data->thread_number, data->thread_count);
  return NULL;
}

// Función principal
int main() {
  size_t thread_count = 10;
  // Crear hilos y obtener el arreglo de datos privados
  private_data* team = create_threads(thread_count, routine, NULL);
  if (team != NULL) {
    join_threads(thread_count, team); // Esperar a que todos los hilos terminen
  }
  return 0;
}