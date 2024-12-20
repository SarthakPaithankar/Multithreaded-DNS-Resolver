#ifndef MULTI_LOOKUP_H
#define MULTI_LOOKUP_H
#define MAX_INPUT_FILES 100
#define MAX_REQUESTOR_THREADS 10
#define MAX_IP_LENGTH INET6_ADDRSTRLEN
#include <pthread.h>
#include <semaphore.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "array.h"
typedef struct {
    pthread_mutex_t* file_mutex;
    pthread_mutex_t* write_mutex;
    pthread_mutex_t* conversion_write_mutex;
    int file_count;
    char *res;
} sems;

typedef struct{
    char **read_files;
    FILE *requestor_log;
    FILE *converted_log;
    int num_args;
    sems *sem;
    array *s;
}requester_args;

int sem_initiator(sems* sem);
char* get_filename(char **files, int num_args,sems *sem);
int record_parse(char *domain, FILE *requestor_log, sems *sem, array *s);
void* requestor(void *requestor_params);


 
#endif
