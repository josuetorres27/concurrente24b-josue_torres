// Josué Torres Sibaja C37853

#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>

typedef struct {
  pthread_mutex_t mutex; // Mutex para proteger el acceso a la piñata
  int hits_left; // Cantidad de golpes que la piñata puede soportar
  int breaker; // ID del hilo que rompió la piñata
} SharedData;

typedef struct {
  int id; // Identificador del hilo
  int hits; // Contador de golpes
  SharedData* shared_data; // Puntero a los datos compartidos
} ThreadData;

void* hit_pinata(void* arg) {
  ThreadData* data = (ThreadData*) arg;
  SharedData* shared = data->shared_data; // Acceso a los datos compartidos
  data->hits = 0; // Inicializar el contador de golpes del hilo

  while (1) {
    // Bloquear el acceso a la piñata
    pthread_mutex_lock(&shared->mutex);

    if (shared->hits_left > 0) {
      shared->hits_left--; // Reducir los golpes restantes
      data->hits++; // Aumentar los golpes del hilo
      printf("Thread %d hit the pinata. Remaining hits: %d\n", data->id,
        shared->hits_left);

      // Si la piñata se rompe
      if (shared->hits_left == 0) {
        shared->breaker = data->id;
        printf("Thread %d broke the pinata!\n", data->id);
        pthread_mutex_unlock(&shared->mutex); // Liberar el mutex
        return NULL;
      }
    } else {
      // Liberar el mutex si ya no hay golpes restantes
      pthread_mutex_unlock(&shared->mutex);
      return NULL;
    }
    // Liberar el mutex para que otro hilo golpee
    pthread_mutex_unlock(&shared->mutex);
  }

  return NULL;
}

int main(int argc, char* argv[]) {
  if (argc != 3) {
    printf("Usage: %s <thread count> <number of hits the pinata can 
      withstand>\n", argv[0]);
    return EXIT_FAILURE;
  }

  int thread_count = atoi(argv[1]); // Número de hilos
  int hits_left = atoi(argv[2]); // Golpes que soporta la piñata

  pthread_t threads[thread_count];
  ThreadData thread_data[thread_count];
  SharedData shared_data;

  // Inicializar los datos compartidos
  pthread_mutex_init(&shared_data.mutex, NULL);
  shared_data.hits_left = hits_left;
  shared_data.breaker = -1; // Aún no se rompe la piñata

  // Crear los hilos
  for (int i = 0; i < thread_count; i++) {
    thread_data[i].id = i;
    // Pasar la referencia a los datos compartidos
    thread_data[i].shared_data = &shared_data;
    pthread_create(&threads[i], NULL, hit_pinata, &thread_data[i]);
  }

  // Esperar a todos los hilos
  for (int i = 0; i < thread_count; i++) {
    pthread_join(threads[i], NULL);
  }

  // Reportar cuántos golpes hizo cada hilo
  for (int i = 0; i < thread_count; i++) {
    printf("Thread %d/%d: %d hits\n", thread_data[i].id, thread_count,
      thread_data[i].hits);
  }

  // Limpiar el mutex
  pthread_mutex_destroy(&shared_data.mutex);

  return 0;
}
