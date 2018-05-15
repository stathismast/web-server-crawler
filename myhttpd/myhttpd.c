#include "server.h"

char rootDir[50] = "/mnt/d/DI/linux/home/syspro/third/script/root_dir";
int numberOfThreads = 2;

pthread_mutex_t mtx;
pthread_cond_t cond_nonempty;
Queue * queue;

pthread_t * pool;

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
    queue = NULL;
    addToQueue(1,&queue);
    popFromQueue(&queue);


	pthread_mutex_init(&mtx, 0);
	pthread_cond_init(&cond_nonempty, 0);

    //Create thread pool
    pool = malloc(numberOfThreads * sizeof(pthread_t));
    for(int i=0; i<numberOfThreads; i++){
        pool[i] = createThread(worker, i);
    }

    sock = createSocket();
    listenForConnections(sock, port);
    while(1){
        acceptConnection(sock);
    }


	pthread_cond_destroy(&cond_nonempty);
	pthread_mutex_destroy(&mtx);
	freeQueue(queue);
}
