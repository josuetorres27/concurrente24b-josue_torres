// Josué Torres Sibaja / C37853 / 2024
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <time.h>

// Definición de la función para generar los números
void* lottery_numbers(void* arg);

// Hilo principal
int main(void) {
  // Crear los hilos secundarios de los nietos
  pthread_t grandson1, grandson2;
  int error1 = pthread_create(&grandson1, NULL, lottery_numbers, NULL);
  int error2 = pthread_create(&grandson2, NULL, lottery_numbers, NULL);
  // Comprobar si los hilos se crearon correctamente
  if (error1 == 0 && error2 == 0) {
    void* number1 = NULL;
    void* number2 = NULL;
    // Esperar a que los hilos terminen y obtener sus números
    pthread_join(grandson1, &number1);
    pthread_join(grandson2, &number2);
  } else {
    fprintf(stderr, "Error creating secondary threads\n");
  }
  return 0;
}