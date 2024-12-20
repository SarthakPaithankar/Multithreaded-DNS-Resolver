#include "multi-lookup.h"
#include "util.h"
int sem_initiator(sems* sem){
    sem->file_mutex = malloc(sizeof(pthread_mutex_t));
    sem->write_mutex = malloc(sizeof(pthread_mutex_t));
    sem->conversion_write_mutex = malloc(sizeof(pthread_mutex_t));
    pthread_mutex_init(sem->file_mutex, NULL);
    pthread_mutex_init(sem->write_mutex, NULL);
    pthread_mutex_init(sem->conversion_write_mutex, NULL);
    sem->file_count = 0;
    sem->res = NULL;
    return 0;
}

//Files should be accessed individually by threads 
//so we can use mutexes to ensure exclusivity
char* get_filename(char **files, int num_args,sems *sem){
    pthread_mutex_lock(sem->file_mutex);   //lock the list of File names
    printf("NUM ARGS: %d \nand FILE COUNT: %d\n", num_args, sem->file_count);
    if(sem->file_count+5 < num_args){        //if there are file available
        sem->res = files[sem->file_count];  
        sem->file_count++;
        printf("Thread %ld processing: %s\n", (unsigned long)pthread_self(),sem->res);       
    }else{
        sem->res = NULL;
    }
    pthread_mutex_unlock(sem->file_mutex); //unlock the list of file names for other threads
    printf("FILE NAME BEFORE GET: %s\n", sem->res);
    return sem->res;
}

//Only one thread should be able to write to the writer file
int record_parse(char *domain, FILE *requestor_log, sems *sem, array *s){
    pthread_mutex_lock(sem->write_mutex);  //locks the requestor log and the shared buffer 
    printf("DOMAIN FROM RECORD: %s\n", domain);
    if(!requestor_log){
        printf("file cant be written to ");
        return 1;
    }
    fprintf(requestor_log, "%s\n",domain);   //write to the log file
    fflush(requestor_log);              //immediate write to the file
    array_put(s, domain);             //write to the shared buffer 
    pthread_mutex_unlock(sem->write_mutex); //unlock the mutex
    return 0;
}

void* requestor(void *requested_par){
    int files_processed = 0;
    requester_args *args = (requester_args*) requested_par;
    char *char_num = malloc(MAX_NAME_LENGTH);
    while(1){
        char *file = get_filename(args->read_files, args->num_args, args->sem);
        if(file == NULL){
            break;
        }
        FILE *opened_file = fopen(file,"r");    //checking for file permissions
        if(opened_file == NULL){
            files_processed += 1;
            break;
        }
        while(fscanf(opened_file, "%s",char_num) != EOF){
            char *domain = strdup(char_num);
            record_parse(domain, args->requestor_log, args->sem, args->s);
        }
        files_processed += 1;
        fclose(opened_file);
    }
    printf("thread %lu serviced %d files\n", (unsigned long)pthread_self(), files_processed);
    free(char_num);
    free(requested_par);
    return NULL;
}
int conversion_log(char *domain, char *IP, FILE *conversion_log, sems *sem){
    pthread_mutex_lock(sem->conversion_write_mutex);  //locks the requestor log and the shared buffer 
    printf("DOMAIN FROM CONVERSION: %s\n", domain);
    fprintf(conversion_log, "%s, %s\n", domain, IP);   //write to the log file
    fflush(conversion_log);              //immediate write to the file
    pthread_mutex_unlock(sem->conversion_write_mutex); //unlock the mutex
    return 1;
}
void* resolver(void *req_pars){
    requester_args *args = (requester_args*)req_pars;
    char *IP_ADD = malloc(MAX_IP_LENGTH);
    int resolved = 0;
    while(1){
        char *removed_file = NULL;
        array_get(args->s, &removed_file);
        if(removed_file == NULL){       //posion pill found we can break the loop
            free(removed_file);
            break;
        }
        if(dnslookup(removed_file, IP_ADD, MAX_IP_LENGTH) == UTIL_FAILURE){
            char *fail = "NOT_RESOLVED";
;           conversion_log(removed_file,fail, args->converted_log, args->sem);
            fflush(args->converted_log);     
        }else{
            conversion_log(removed_file, IP_ADD, args->converted_log, args->sem);
            resolved += 1;
        }
        free(removed_file);
    }
    free(IP_ADD);
    free(req_pars);
    printf("thread %lu resolved %d hostnames", pthread_self(), resolved);
    return NULL;
}


int main(int argc, char **argv){
    if(argc < 6){
        printf("Not Enough Arguments");
        exit(1);
    }
    if(atoi(argv[1]) > 10 || atoi(argv[2]) > 10 ){
        printf("Too many requestor/resolver threads");
        exit(1);
    }
    if(argc > 105){
        printf("Too many input files");
        exit(1);
    }
    sems *s = malloc(sizeof(sems));
    sem_initiator(s);
    array *shared_buffer = malloc(sizeof(array)); 
    array_init(shared_buffer);
    FILE *requestor_file = fopen(argv[3], "w");
    FILE *converted_file = fopen(argv[4], "a");
    pthread_t requestor_threads[atoi(argv[1])];
    pthread_t resolver_threads[atoi(argv[2])];
    requester_args *args = malloc(sizeof(requester_args));



    args->read_files = argv+5;
    args->requestor_log = requestor_file;
    args->converted_log = converted_file;
    args->num_args = argc;
    args->sem = s;
    args->s = shared_buffer;
    
    requester_args *reqthread_args[atoi(argv[1])];
    requester_args *resthread_args[atoi(argv[2])];
    
    for(int i = 0;i < atoi(argv[1]);i++){
        reqthread_args[i] = malloc(sizeof(requester_args));
        *reqthread_args[i] = *args;
        pthread_create(&requestor_threads[i], NULL, requestor, (void *)reqthread_args[i]);
    }
    for(int i = 0;i < atoi(argv[2]);i++){
        resthread_args[i] = malloc(sizeof(requester_args));
        *resthread_args[i] = *args;
        pthread_create(&resolver_threads[i], NULL, resolver, (void *)resthread_args[i]);
    }
    for(int i = 0;i < atoi(argv[1]);i++){
        pthread_join(requestor_threads[i], NULL);
    }
    //Insert the Poison pills
    for(int i = 0;i < atoi(argv[2]);i++){
        array_put(shared_buffer, NULL);
    }
    for(int i = 0; i < atoi(argv[2]);i++){
        pthread_join(resolver_threads[i], NULL);
    }
    printf("%lu", sizeof(requester_args));
    fclose(converted_file);
    fclose(requestor_file);
    printf("done");
    free(s->conversion_write_mutex);
    free(s->file_mutex);
    free(s->write_mutex);
    free(shared_buffer);
    free(args);
    free(s);
}

