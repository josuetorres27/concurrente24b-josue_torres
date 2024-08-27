// Josué Torres Sibaja / C37853 / 2024
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>

// Definición de las funciones para generar los números
void* number1(void* arg);
void* number2(void* arg);

// Variante 1

void* number1(void* arg) {
  int* num1 = malloc(sizeof(int)); // Reservar la memoria para el número random
  if (num1 == NULL) {
    fprintf(stderr, "Error allocating memory for number 1\n");
    pthread_exit(NULL);
  }
  *num1 = rand() % 100; // Generar número random entre 00 y 99
  pthread_exit((void*) num1); // Retornar la dirección de memoria
}

void* number2(void* arg) {
  int* num2 = malloc(sizeof(int)); // Reservar la memoria para el número random
  if (num2 == NULL) {
    fprintf(stderr, "Error allocating memory for number 2\n");
    pthread_exit(NULL);
  }
  *num2 = rand() % 100; // Generar número random entre 00 y 99
  pthread_exit((void*) num2); // Retornar la dirección de memoria
}

// Variante 2

void* number1_false_adress(void* arg) {
    int num1 = rand() % 100; // Número random entre 00 y 99
    return (void*)(intptr_t) num1; // Convertir el número a una falsa dirección de memoria
}

void* number2_false_adress(void* arg) {
    int num2 = rand() % 100; // Número random entre 00 y 99
    return (void*)(intptr_t) num2; // Convertir el número a una falsa dirección de memoria
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
  printf("El número obtenido por el nieto 2 (variante 1) es: %d\n", *(int*)number2);

  // Liberar la memoria utilizada
  free(number1);
  free(number2);

  // Crear hilos para la variante 2
  pthread_create(&grandch1, NULL, number1_false_adress, NULL);
  pthread_create(&grandch2, NULL, number2_false_adress, NULL);

  // Esperar a que terminen los hilos y recoger los resultados
  pthread_join(grandch1, &number1);
  pthread_join(grandch2, &number2);

  // Imprimir los resultados convertidos de la "falsa dirección de memoria"
  printf("El número obtenido por el nieto 1 (variante 2) es: %d\n", *(int*)number1);
  printf("El número obtenido por el nieto 2 (variante 2) es: %d\n", *(int*)number2);

  return 0;
}