// Copyright 2024 Esteban Solís <esteban.solisalfaro@ucr.ac.cr>
// Copyright 2024 Josué Torres <josue.torressibaja@ucr.ac.cr>
#include "task_handler.h"

void* do_walls(void* data) {
    assert(data);
    // Access to shared data
    private_data_t* private_data = (private_data_t*) data;
    shared_data_t* shared_data = private_data->shared_data;
    printf("Walls started\n");
    sleep(lrand48() % 10);
    printf("\t\tWalls finished\n");
    // Two sem_post because there are two threads waiting for this information.
    sem_post(&shared_data->walls_ready);
    sem_post(&shared_data->walls_ready);
    return NULL;
}

void* do_roof(void* data) {
    assert(data);
    private_data_t* private_data = (private_data_t*) data;
    shared_data_t* shared_data = private_data->shared_data;
    sem_wait(&shared_data->walls_ready);
    printf("Roof started\n");
    sleep(lrand48() % 10);
    printf("\t\tRoof finished\n");
    // There's only one thread waiting for this info.
    sem_post(&shared_data->roof_ready);
    return NULL;
}

void* do_exterior_plumbing(void* data) {
    assert(data);
    private_data_t* private_data = (private_data_t*) data;
    shared_data_t* shared_data = private_data->shared_data;
    sem_wait(&shared_data->walls_ready);
    printf("Exterior plumbing started\n");
    sleep(lrand48() % 10);
    printf("\t\tExterior plumbing finished\n");
    sem_post(&shared_data->exterior_plumbing_ready);
    sem_post(&shared_data->exterior_plumbing_ready);
    return NULL;
}

void* do_electrical_installation(void* data) {
    assert(data);
    private_data_t* private_data = (private_data_t*) data;
    shared_data_t* shared_data = private_data->shared_data;
    sem_wait(&shared_data->roof_ready);
    printf("Electrical installation started\n");
    sleep(lrand48() % 10);
    printf("\t\tElectrical installation finished\n");
    sem_post(&shared_data->electrical_installation_ready);
    return NULL;
}

void* do_interior_plumbing(void* data) {
    assert(data);
    private_data_t* private_data = (private_data_t*) data;
    shared_data_t* shared_data = private_data->shared_data;
    sem_wait(&shared_data->exterior_plumbing_ready);
    printf("Interior plumbing started\n");
    sleep(lrand48() % 10);
    printf("\t\tInterior plumbing finished\n");
    sem_post(&shared_data->interior_plumbing_ready);
    return NULL;
}

void* do_interior_painting(void* data) {
    assert(data);
    private_data_t* private_data = (private_data_t*) data;
    shared_data_t* shared_data = private_data->shared_data;
    sem_wait(&shared_data->interior_plumbing_ready);
    sem_wait(&shared_data->electrical_installation_ready);
    printf("Interior painting started\n");
    sleep(lrand48() % 10);
    printf("\t\tInterior painting finished\n");
    sem_post(&shared_data->interior_painting_ready);
    return NULL;
}
void* do_exterior_painting(void* data) {
    assert(data);
    private_data_t* private_data = (private_data_t*) data;
    shared_data_t* shared_data = private_data->shared_data;
    sem_wait(&shared_data->exterior_plumbing_ready);
    printf("Exterior painting started\n");
    sleep(lrand48() % 10);
    printf("\t\tExterior painting finished\n");
    sem_post(&shared_data->exterior_painting_ready);
    return NULL;
}

void* do_floor(void* data) {
    assert(data);
    private_data_t* private_data = (private_data_t*) data;
    shared_data_t* shared_data = private_data->shared_data;
    sem_wait(&shared_data->interior_painting_ready);
    printf("Floor started\n");
    sleep(lrand48() % 10);
    printf("\t\tFloor finished\n");
    sem_post(&shared_data->floor_ready);
    return NULL;
}
void* do_interior_finishes(void* data) {
    assert(data);
    private_data_t* private_data = (private_data_t*) data;
    shared_data_t* shared_data = private_data->shared_data;
    sem_wait(&shared_data->floor_ready);
    printf("Interior finishes started\n");
    sleep(lrand48() % 10);
    printf("\t\tInterior finishes finished\n");
    return NULL;
}
void* do_exterior_finishes(void* data) {
    assert(data);
    private_data_t* private_data = (private_data_t*) data;
    shared_data_t* shared_data = private_data->shared_data;
    sem_wait(&shared_data->exterior_painting_ready);
    printf("Exterior finishes started\n");
    sleep(lrand48() % 10);
    printf("\t\tExterior finishes finished\n");
    return NULL;
}
