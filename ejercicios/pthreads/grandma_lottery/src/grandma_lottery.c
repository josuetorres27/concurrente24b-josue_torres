// Josué Torres Sibaja / C37853 / 2024
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>

// Definición de las funciones para generar los números
void* number1(void* arg);
void* number2(void* arg);

// Variante 1

void* number1(void* arg) {
  int* number1 = malloc(sizeof(int)); // Reservar la memoria para el número random
  if (number1 == NULL) {
    fprintf(stderr, "Error allocating memory for number 1\n");
    pthread_exit(NULL);
  }
  *number1 = rand() % 100; // Generar número random entre 00 y 99
  pthread_exit((void*) number1); // Retornar la dirección de memoria
}

void* number2(void* arg) {
  int* number2 = malloc(sizeof(int)); // Reservar la memoria para el número random
  if (number2 == NULL) {
    fprintf(stderr, "Error allocating memory for number 2\n");
    pthread_exit(NULL);
  }
  *number2 = rand() % 100; // Generar número random entre 00 y 99
  pthread_exit((void*) number1); // Retornar la dirección de memoria
}

// Hilo principal

int main() {
  pthread_t grandch1, grandch2; // Hilos de los nietos
  void* number1; // Para almacenar el valor retornado por el nieto 1
  void* number2; // Para almacenar el valor retornado por el nieto 2

  srand(time(NULL)); // Semilla para los números aleatorios

  // Crear hilos para la variante 1
  pthread_create(&grandch1, NULL, number1, NULL);
  pthread_create(&grandch2, NULL, number2, NULL);

  // Esperar a que terminen los hilos y recoger los resultados
  pthread_join(grandch1, &number1);
  pthread_join(grandch2, &number2);

  // Imprimir los resultados
  printf("El número obtenido por el nieto 1 (variante 1) es: %d\n", *(int*)number1);
  printf("El número obtenido por el nieto 2 (variante 2) es: %d\n", *(int*)number2);

  // Liberar la memoria utilizada
  free(number1);
  free(number2);
}