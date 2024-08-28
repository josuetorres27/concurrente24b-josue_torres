// Josué Torres Sibaja / C37853 / 2024
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <time.h>

// Definición de la subrutina para generar los números
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

    // Imprimir los números obtenidos

    /* Variante que da problemas al retornar la dirección de memoria de los números:
    ** printf("Número obtenido por el nieto 1: %d\n", *(int*) number1);
    ** printf("Número obtenido por el nieto 2: %d\n", *(int*) number2);
    */

    // Variante que produce resultados correctos:
    printf("Número obtenido por el nieto 1: %zu\n", (size_t) number1);
    printf("Número obtenido por el nieto 2: %zu\n", (size_t) number2);
  } else {
    fprintf(stderr, "Error creating secondary threads\n");
  }
  return 0;
}

// Subrutina para generar los números
void* lottery_numbers(void* arg) {
  (void) arg; // Evitar el warning de argumento no utilizado

  // Generar número pseudoaleatorio entre 0 y 99
  unsigned semilla = time(NULL) + clock();
  size_t num = rand_r(&semilla) % 100;

  /* Se retorna la dirección de memoria del número generado (genera problemas):
  ** return &num;
  */

  // Se retorna una falsa dirección de memoria (genera resultados correctos):
  return (void*) num;
}