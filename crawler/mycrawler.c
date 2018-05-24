#include "crawler.h"

char * host;
int port;

Queue * queue;
Queue * nextFile;
pthread_mutex_t mtx;
pthread_cond_t cond_nonempty;

int numberOfThreads = 10;
pthread_t * threadPool;

int done = 0;

int main(int argc, char *argv[]){

    manageArguments(argc,argv);

    queue = NULL;   //Initialize queue to NULL
    addToQueue("/site2/page7_16864.html",&queue);
    nextFile = queue;

    pthread_mutex_init(&mtx, 0);
	pthread_cond_init(&cond_nonempty, 0);

    // pthread_t thread = createThread(worker, 0);
    //Create threadPool
    threadPool = malloc(numberOfThreads * sizeof(pthread_t));
    for(int i=0; i<numberOfThreads; i++){
        threadPool[i] = createThread(worker, i);
    }

    // pthread_join(thread,0);
    for(int i=0; i<numberOfThreads; i++){
        joinThread(threadPool[i]);
    }


    printQueue(queue);
    freeQueue(queue);
}                     /* Close socket and exit */
