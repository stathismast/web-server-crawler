#include "threads.h"

#define perrorThreads(s,e) fprintf(stderr, "%s: %s\n", s, strerror(e))

pthread_t createThread(void *(*start_routine) (void *), int arg){
    pthread_t thread;
    int * argp = malloc(sizeof(int));
    *argp = arg;
    int err;

    if (err = pthread_create(&thread, NULL, start_routine, (void *)argp)) { /* New thread */
        perrorThreads("pthread_create", err);
        exit(1);
    }
    // printf("%ld: Created thread %ld\n", pthread_self(), thread);
    return thread;
}

void joinThread(pthread_t thread){
    int err, status;
    if (err = pthread_join(thread, (void **) &status)) { /* Wait for thread */
        perrorThreads("pthread_join", err); /* termination */
        exit(1);
    }
    // printf("Thread %ld exited with code %d\n", thread, status);
}
