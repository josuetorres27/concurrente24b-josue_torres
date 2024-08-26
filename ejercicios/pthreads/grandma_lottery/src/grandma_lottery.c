// Josué Torres Sibaja / C37853 / 2024
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>

// Definición de las funciones de los nietos
void* grandch1(void* arg);
void* grandch2(void* arg);

// *** Variante 1 ***

void* grandch1(void* arg) {
  int* number1 = malloc(sizeof(int)); // Reserva de memoria
  if (number1 == NULL) {
    fprintf(stderr, "Error allocating memory for number 1\n");
    pthread_exit(NULL);
  }
  *number1 = rand() % 100; // Número random entre 00 y 99
  pthread_exit((void*) number1); // Retorna la dirección de memoria
}

void* grandch2(void* arg) {
  int* number2 = malloc(sizeof(int)); // Reserva de memoria
  if (number2 == NULL) {
    fprintf(stderr, "Error allocating memory for number 2\n");
    pthread_exit(NULL);
  }
  *number2 = rand() % 100; // Número random entre 00 y 99
  pthread_exit((void*) number2); // Retorna la dirección de memoria
}