// Josué Torres Sibaja / C37853 / 2024
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>

void* greet(void* size_t_number);

int main(void) {
  pthread_t thread;

  // Declarar una variable local de tipo size_t e inicializarla en 2
  size_t number = 2;

  // Pasar un puntero al número al hilo que ejecuta la función greet()
  int error = pthread_create(&thread, /*attr*/ NULL, greet, &number);

  if (error == 0) {
    printf("Hello from main thread\n");
    pthread_join(thread, /*value_ptr*/ NULL);
  } else {
    fprintf(stderr, "Error: could not create secondary thread\n");
    return error;
  }
}

void* greet(void* size_t_number) {
  // Convertir el argumento recibido a size_t*
  size_t* number_ptr = (size_t*)size_t_number;

  // Obtener el valor del número
  size_t number = *number_ptr;

  if (number == 0) {
    // Imprimir una despedida si el número es 0
    printf("Goodbye! Received number: %zu\n", number);
  } else {
    // Imprimir un saludo si el número es mayor que 0
    printf("Hello from secondary thread. Received number: %zu\n", number);

    // Crear un nuevo hilo con un número menor al recibido
    pthread_t new_thread;
    size_t new_number = number - 1;
    int error = pthread_create(&new_thread, /*attr*/ NULL, greet, &new_number);

    // Verificar si el nuevo hilo se creó correctamente
    if (error == 0) {
      pthread_join(new_thread, /*value_ptr*/ NULL);
    } else {
      fprintf(stderr, "Error: could not create new thread\n");
    }
  }
  return NULL;
}