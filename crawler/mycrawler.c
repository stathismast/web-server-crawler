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

void handler(){
    printf("I am the handler\n");
    done = 1;
}

//Temporarily using pipes unti I implement the command socket
void createNamedPipe(char * pipeName){
    unlink(pipeName);
    if(mkfifo(pipeName, 0600) == -1){
        perror("sender: mkfifo");
        exit(6);
    }
}

int main(int argc, char *argv[]){

    manageArguments(argc,argv);

    queue = NULL;   //Initialize queue to NULL
    addToQueue("/site2/page7_16864.html",&queue);
    nextFile = queue;

    pthread_mutex_init(&mtx, 0);
	pthread_cond_init(&cond_nonempty, 0);


    createNamedPipe("/tmp/thisWouldBeTheSocket");
    int tempFD = open("/tmp/thisWouldBeTheSocket", O_RDWR);


    // pthread_t thread = createThread(worker, 0);
    //Create threadPool
    signal(SIGUSR1,SIG_IGN);
    threadPool = malloc(numberOfThreads * sizeof(pthread_t));
    for(int i=0; i<numberOfThreads; i++){
        threadPool[i] = createThread(worker, i);
    }
    signal(SIGUSR1,handler);

    struct pollfd * fds = malloc(sizeof(struct pollfd));
    fds->fd = tempFD;
    fds->events = POLLIN;
    fds->revents = 0;

    while(!done)
        if(poll(fds,1,2000) != 1) { printf("poll is done\n"); if(done) break; }
        else printf("Inform the command port that we are not done crawling\n");
    // printf("Crawling has finished\n");

    // pthread_join(thread,0);
    for(int i=0; i<numberOfThreads; i++){
        pthread_cond_broadcast(&cond_nonempty);
        joinThread(threadPool[i]);
    }

    free(fds);
    free(threadPool);

    printQueue(queue);
    freeQueue(queue);
}                     /* Close socket and exit */
