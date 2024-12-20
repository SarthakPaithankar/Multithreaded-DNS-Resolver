#ifndef ARRAY_H
#define ARRAY_H
#define ARRAY_SIZE 8                       // max elements in array
#define MAX_NAME_LENGTH 30
#include <pthread.h>
#include <semaphore.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef struct {
    char *array[ARRAY_SIZE]; // storage array for pointers to strings
    int head;                                // array index indicating where the top is
    int tail;
    pthread_mutex_t mutex;
    sem_t space_available_semaphore;
    sem_t items_available_semaphore;                                // array index indicating where the bottom is
    sem_t critical_block_semaphore;
} array;

int  array_init(array *s);                   // initialize the array
int  array_put (array *s, char *hostname);   // place element into the array, block when full
int  array_get (array *s, char **hostname);  // remove element from the array, block when empty
//void array_free(array *s);                   // free the array's resources

#endif