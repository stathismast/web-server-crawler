#include "server.h"

char * rootDir;
int numberOfThreads = 10;

pthread_mutex_t mtx;
pthread_cond_t cond_nonempty;
Queue * queue;

pthread_t * threadPool;

int done = 0;

int main(int argc, char *argv[]){
    int sock;
    int port;

    //Manage arguments
    if (argc < 3){
        printf("Please give the port number and root directory\n");
        exit(1);
    }
    port = atoi(argv[1]);
    rootDir = malloc(strlen(argv[2])+1);
    bzero(rootDir, strlen(argv[2])+1); //Initialize string
    strcpy(rootDir, argv[2]);

    //Initialize queue
    initQueue(&queue);

	pthread_mutex_init(&mtx, 0);
	pthread_cond_init(&cond_nonempty, 0);

    //Create threadPool
    threadPool = malloc(numberOfThreads * sizeof(pthread_t));
    for(int i=0; i<numberOfThreads; i++){
        threadPool[i] = createThread(worker, i);
    }

    sock = createSocket();
    listenForConnections(sock, port);
    while(1){
        acceptConnection(sock);
    }

    //Clean up and exit
	pthread_cond_destroy(&cond_nonempty);
	pthread_mutex_destroy(&mtx);
	freeQueue(queue);
}
