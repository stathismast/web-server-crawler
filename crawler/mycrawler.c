#include "crawler.h"

char * host;
int port;
int commandPort;
int commandSocket;
char * startingURL;

Queue * queue;
Queue * nextFile;
pthread_mutex_t mtx;
pthread_cond_t cond_nonempty;

int numberOfThreads = 20;
pthread_t * threadPool;

char saveDir[64] = "save_dir";

struct timeval startingTime;       //Start time in seconds

int done = 0;

void handler(){
    printf("I am the handler\n");
    done = 1;
}

int main(int argc, char *argv[]){
    gettimeofday(&startingTime, NULL);

    manageArguments(argc,argv);

    queue = NULL;   //Initialize queue to NULL
    addToQueue(startingURL,&queue);
    nextFile = queue;

    pthread_mutex_init(&mtx, 0);
	pthread_cond_init(&cond_nonempty, 0);

    // pthread_t thread = createThread(worker, 0);
    //Create threadPool
    signal(SIGUSR1,SIG_IGN);
    threadPool = malloc(numberOfThreads * sizeof(pthread_t));
    for(int i=0; i<numberOfThreads; i++){
        threadPool[i] = createThread(worker, i);
    }
    signal(SIGUSR1,handler);

    struct pollfd * commandPortFD = malloc(sizeof(struct pollfd));
    commandSocket = createSocket();
    listenForConnections(commandSocket, commandPort);
    commandPortFD->fd = commandSocket;
    commandPortFD->events = POLLIN;
    commandPortFD->revents = 0;

    createDirectory(saveDir);

    while(!done)
        if(poll(commandPortFD,1,1000) != 1) { if(done) break; }
        else{
            printf("Inform the command port that we are not done crawling\n");
            acceptConnectionWhileCrawling(commandSocket);
            commandPortFD->revents = 0;
        }
    // printf("Crawling has finished\n");

    // pthread_join(thread,0);
    for(int i=0; i<numberOfThreads; i++){
        pthread_cond_broadcast(&cond_nonempty);
        joinThread(threadPool[i]);
    }

    free(threadPool);

    printQueue(queue);

    unlink("index.txt");
    FILE *stream = fopen("index.txt", "ab+");
    Queue * node = queue;
    Queue * dirQueue = NULL;
    while(node != NULL){
        addToQueue(strtok(node->fileName,"/"),&dirQueue);
        node = node->next;
    }
    node = dirQueue;
    while(node != NULL){
        fprintf(stream,"%s/",saveDir);
        fprintf(stream,"%s\n",node->fileName);
        node = node->next;
    }
    fclose(stream);

    freeQueue(queue);
    freeQueue(dirQueue);
    free(startingURL);

    jobExecutor("index.txt",4);

    done = 0;
    commandPortFD->revents = 0;
    while(!done)
        if(poll(commandPortFD,1,1000) != 1) { if(done) break; }
        else{
            acceptCommandConnection(commandSocket);
            commandPortFD->revents = 0;
        }

    cleanUp();
    free(commandPortFD);
    unlink("index.txt");
}                     /* Close socket and exit */
