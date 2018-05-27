#include "server.h"

char * rootDir;

pthread_mutex_t mtx;
pthread_cond_t cond_nonempty;
Queue * queue;

int numberOfThreads = 10;
pthread_t * threadPool;

int done = 0;

struct timeval startingTime;       //Start time in seconds

int main(int argc, char *argv[]){
    gettimeofday(&startingTime, NULL);

    int servSocket;
    int servPort;
    int commSocket;
    int commPort;

    //Manage arguments
    if (argc < 3){
        printf("Please give the server command port, command port number and root directory\n");
        exit(1);
    }
    servPort = atoi(argv[1]);
    commPort = atoi(argv[2]);
    rootDir = malloc(strlen(argv[3])+1);
    bzero(rootDir, strlen(argv[3])+1); //Initialize string
    strcpy(rootDir, argv[3]);

    //Initialize queue
    initQueue(&queue);

	pthread_mutex_init(&mtx, 0);
	pthread_cond_init(&cond_nonempty, 0);

    //Create threadPool
    threadPool = malloc(numberOfThreads * sizeof(pthread_t));
    for(int i=0; i<numberOfThreads; i++){
        threadPool[i] = createThread(worker, i);
    }


    struct pollfd * sockets = malloc(2*sizeof(struct pollfd));

    servSocket = createSocket();
    listenForConnections(servSocket, servPort);
    sockets[0].fd = servSocket;
    sockets[0].events = POLLIN;
    sockets[0].revents = 0;

    commSocket = createSocket();
    listenForConnections(commSocket, commPort);
    sockets[1].fd = commSocket;
    sockets[1].events = POLLIN;
    sockets[1].revents = 0;

    int requests = 0;
    while(!done){
        requests = poll(sockets,2,1000);
        if(requests > 0){
            if(sockets[0].revents & POLLIN){
                acceptConnection(servSocket);
                sockets[0].revents = 0;
            }
            if(sockets[1].revents & POLLIN){
                acceptCommandConnection(commSocket);
                sockets[1].revents = 0;
            }
        }
    }

    pthread_cond_broadcast(&cond_nonempty);
    for(int i=0; i<numberOfThreads; i++){
        joinThread(threadPool[i]);
    }

    //Clean up and exit
	pthread_cond_destroy(&cond_nonempty);
	pthread_mutex_destroy(&mtx);
	freeQueue(queue);
    free(sockets);
    free(threadPool);
    free(rootDir);
}
