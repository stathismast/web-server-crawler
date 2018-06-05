#include "server.h"

char * rootDir;     //Root directory
int servPort;       //Server port
int commPort;       //Command post

pthread_mutex_t mtx;            //Mutex to lock queue
pthread_cond_t cond_nonempty;   //Condition signaling that the queue is nonempty
Queue * queue;                  //Queue with pending requests

int numberOfThreads;            //Total number of threads
pthread_t * threadPool;         //Pool of threads

int done = 0;                   //Boolean variable used to terminate threads

int verbose = 0;                //Boolean variable to enable verbose output

struct timeval startingTime;       //Start time in seconds

int main(int argc, char *argv[]){
    int servSocket;
    int commSocket;

    gettimeofday(&startingTime, NULL);  //Get starting time

    //Manage arguments
    manageArguments(argc,argv);

    //Initialize queue
    initQueue(&queue);

    //Initialize mutex and condition variable
	pthread_mutex_init(&mtx, 0);
	pthread_cond_init(&cond_nonempty, 0);

    //Create threadPool
    threadPool = malloc(numberOfThreads * sizeof(pthread_t));
    for(int i=0; i<numberOfThreads; i++){
        threadPool[i] = createThread(worker, i);
    }

    //Create pollfd structs for both the server and the command socket
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

    printf("Server up and running...\n");

    //Poll for GET requests (on server port) or commands (on command port)
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

    //When we get to this point, variable 'done' has a value of 1, so we
    //Signal the threads to exit and join
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
