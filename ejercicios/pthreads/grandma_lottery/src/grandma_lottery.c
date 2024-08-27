// Josué Torres Sibaja / C37853 / 2024
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <time.h>

// Definición de las funciones para generar los números
void* variant1(void* arg);
void* variant2(void* arg);

// Hilo principal
int main() {
  pthread_t grandson1;
  pthread_t grandson2;
  void* number1 = NULL;
  void* number2 = NULL;

  srand(time(NULL)); // Inicializar semilla para los números random

  // Crear hilos para la variante 1
  int error1 = pthread_create(&grandson1, NULL, variant1, NULL);
  int error2 = pthread_create(&grandson2, NULL, variant1, NULL);

  if (error1 == 0 && error2 == 0) { // Comprobar si los hilos se crearon correctamente
    pthread_join(grandson1, &number1); // Esperar a que el hilo termine y obtener el número
    printf("Grandson 1 number (variant 1) is: %d\n", *(int*)number1);
    free(number1); // Liberar la memoria asignada al número
    pthread_join(grandson2, &number2);
    printf("Grandson 2 number (variant 1) is: %d\n", *(int*)number2);
    free(number2);
  } else {
    fprintf(stderr, "Error creating threads for variant 1\n");
  }

  // Crear hilos para la variante 2
  error1 = pthread_create(&grandson1, NULL, variant2, NULL);
  error2 = pthread_create(&grandson2, NULL, variant2, NULL);

  if (error1 == 0 && error2 == 0) {
    pthread_join(grandson1, &number1);
    printf("Grandson 1 number (variant 2) is: %d\n", (int)(intptr_t)number1);
    pthread_join(grandson2, &number2);
    printf("Grandson 2 number (variant 2) is: %d\n", (int)(intptr_t)number2);
  } else {
    fprintf(stderr, "Error creating threads for variant 2\n");
  }
  return 0;
}

// Variante 1
void* variant1(void* arg) {
  int* num = (int*)malloc(sizeof(int)); // Reservar la memoria para el número random
  if (num == NULL) { // Comprobar si la memoria se reservó correctamente
    fprintf(stderr, "Error allocating memory for number 1\n");
    pthread_exit(NULL);
  }
  *num = rand() % 100; // Generar número random entre 0 y 99
  pthread_exit((void*)num); // Retornar la dirección de memoria
}

// Variante 2
void* variant2(void* arg) {
  int num = rand() % 100; // Número random entre 0 y 99
  return (void*)(intptr_t)num; // Convertir el número a una falsa dirección de memoria
}