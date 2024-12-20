#include "array.h"

int  array_init(array *s){
    s->head = 0;
    s->tail = 0;
    pthread_mutex_init(&s->mutex, NULL);
    if(sem_init(&s->space_available_semaphore, 0, ARRAY_SIZE) == -1) return 1; 
    if(sem_init(&s->items_available_semaphore, 0, 0) == -1) return 1; 
    if(sem_init(&s->critical_block_semaphore,0,1) == -1) return 1;
    return 0;
}                  
int  array_put (array *s, char *hostname){  //queue
    sem_wait(&s->space_available_semaphore);
    pthread_mutex_lock(&s->mutex);     //lock the critical section of code
    s->array[s->tail] = hostname;
    s->tail = (s->tail+1) % ARRAY_SIZE;
    pthread_mutex_unlock(&s->mutex);
    sem_post(&s->items_available_semaphore);      //Since we added a element to the array we can signal
    return 0;
}  
int  array_get (array *s, char **hostname){ //dequeue
    sem_wait(&s->items_available_semaphore);        // get requests are blocked be blocked
    pthread_mutex_lock(&s->mutex);
    *hostname = s->array[s->head];
    s->head = (s->head+1) % ARRAY_SIZE;
    pthread_mutex_unlock(&s->mutex);
    sem_post(&s->space_available_semaphore);     //Signal the semaphore from put when the array was full;
    return 0;
}