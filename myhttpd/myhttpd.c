#include "server.h"

char rootDir[50] = "/mnt/d/DI/linux/home/syspro/third/script/root_dir";
int numberOfThreads = 2;

pthread_mutex_t mtx;
pthread_cond_t cond_nonempty;
Queue * queue;

pthread_t * threadPool;

int done = 0;

int main(int argc, char *argv[]){
    int sock;
    int port;

    //Manage arguments
    if (argc < 2){
        printf("Please give the port number\n");
        exit(1);
    }
    port = atoi(argv[1]);

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
