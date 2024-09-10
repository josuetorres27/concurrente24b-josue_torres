// Copyright 2024 Esteban Solís <esteban.solisalfaro@ucr.ac.cr>
// Copyright 2024 Josué Torres <josue.torressibaja@ucr.ac.cr>
#include "task_handler.h"

/**
 * @brief Executes the building tasks using multiple threads.
 *
 * @details Allocates memory for threads, initializes
 * semaphores for task synchronization, and creates and joins the threads.
 *
 * @param shared_data Pointer to shared data struct containing semaphores.
 * @param private_data Pointer to private data struct.
 * @return EXIT_SUCCESS on success, error code if failure.
 */
int do_tasks(shared_data_t* shared_data, private_data_t* private_data);

/**
 * @brief Joins the threads after task execution.
 *
 * @param threads Array of threads.
 * @return EXIT_SUCCESS on success, error code if failure.
 */
int join_threads(pthread_t* threads);

/**
 * @brief Allocates memory for shared and private data,
 * and calls the do_tasks() function.
 *
 * @return EXIT_SUCCESS on success, error code otherwise.
 */
int main() {
    int error = EXIT_SUCCESS;
    // Allocate shared data
    shared_data_t* shared_data =
        (shared_data_t*)calloc(1, sizeof(shared_data_t));
    // Allocate private data
    private_data_t* private_data =
        (private_data_t*)calloc(10, sizeof(private_data_t));
    if (shared_data && private_data) {
    /// Create sempahores, threads, and run simulation.
    error = do_tasks(shared_data, private_data);
    } else {
        fprintf(stderr, "Error: could not allocate data\n");
        error = 11;
    }

    free(shared_data);
    free(private_data);
    return error;
}

int do_tasks(shared_data_t* shared_data, private_data_t* private_data) {
    int error = EXIT_SUCCESS;
    // Initialize all semaphores.
    sem_init(&shared_data->walls_ready, 0, 0);
    sem_init(&shared_data->roof_ready, 0, 0);
    sem_init(&shared_data->electrical_installation_ready, 0, 0);
    sem_init(&shared_data->exterior_plumbing_ready, 0, 0);
    sem_init(&shared_data->interior_plumbing_ready, 0, 0);
    sem_init(&shared_data->interior_painting_ready, 0, 0);
    sem_init(&shared_data->exterior_painting_ready, 0, 0);
    sem_init(&shared_data->floor_ready, 0, 0);

    pthread_t* threads = (pthread_t*)
    malloc(10 * sizeof(pthread_t));

    // Assigning private_data to each thread.
    for (uint64_t thread_number = 0; thread_number < 10
        ; ++thread_number) {
      private_data[thread_number].shared_data = shared_data;;
    }

    // Each thread executes its own task.
    if (threads) {
        // walls
        error = pthread_create(&threads[0], /*attr*/ NULL, do_walls
            , /*arg*/ &private_data[0]);
        // roof
        error = pthread_create(&threads[1], /*attr*/ NULL, do_roof
            , /*arg*/ &private_data[1]);
        // electrical installation
        error = pthread_create(&threads[2], /*attr*/ NULL
            , do_electrical_installation, /*arg*/ &private_data[2]);
        // exterior plumbing
        error = pthread_create(&threads[3], /*attr*/ NULL
            , do_exterior_plumbing , /*arg*/ &private_data[3]);
        // interior plumbing
        error = pthread_create(&threads[4], /*attr*/ NULL
            , do_interior_plumbing , /*arg*/ &private_data[4]);
        // interior painting
        error = pthread_create(&threads[5], /*attr*/ NULL
            , do_interior_painting , /*arg*/ &private_data[5]);
        // exterior painting
        error = pthread_create(&threads[6], /*attr*/ NULL
            , do_exterior_painting , /*arg*/ &private_data[6]);
        // floor
        error = pthread_create(&threads[7], /*attr*/ NULL
            , do_floor, /*arg*/ &private_data[7]);
        // interior finishes
        error = pthread_create(&threads[8], /*attr*/ NULL
            , do_interior_finishes, /*arg*/ &private_data[8]);
        // exterior finishes
        error = pthread_create(&threads[9], /*attr*/ NULL
            , do_exterior_finishes, /*arg*/ &private_data[9]);

        error = join_threads(threads);
    } else {
        fprintf(stderr, "Error: could not allocate threads array\n");
        error = 21;
    }

    free(threads);

    // Destroy sempahores
    sem_destroy(&shared_data->walls_ready);
    sem_destroy(&shared_data->roof_ready);
    sem_destroy(&shared_data->electrical_installation_ready);
    sem_destroy(&shared_data->exterior_plumbing_ready);
    sem_destroy(&shared_data->interior_plumbing_ready);
    sem_destroy(&shared_data->interior_painting_ready);
    sem_destroy(&shared_data->exterior_painting_ready);
    sem_destroy(&shared_data->floor_ready);

    return error;
}

int join_threads(pthread_t* threads) {
    int error = EXIT_SUCCESS;
    for (uint64_t thread_number = 0; thread_number < 10
        ; ++thread_number) {
      pthread_join(threads[thread_number], /*value_ptr*/ NULL);
    }
    return error;
}
