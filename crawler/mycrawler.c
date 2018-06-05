#include "crawler.h"

char * host;
int port;
int commandPort;
int commandSocket;
char * startingURL;

Queue * queue;
Queue * nextFile;   //Pointer to the file that should be requested next
pthread_mutex_t mtx;
pthread_cond_t cond_nonempty;

int numberOfThreads;
pthread_t * threadPool;

char * saveDir;

struct timeval startingTime;       //Start time in seconds

int done = 0;

int verbose = 0;

void handler(){
    done = 1;
}

int main(int argc, char *argv[]){
    gettimeofday(&startingTime, NULL);      //Starting time

    manageArguments(argc,argv);

    queue = NULL;   //Initialize queue to NULL
    addToQueue(startingURL,&queue);     //Add the starting URL to the queue
    nextFile = queue;

    //Initialize mutex and cond var
    pthread_mutex_init(&mtx, 0);
	pthread_cond_init(&cond_nonempty, 0);

    //Create threadPool
    signal(SIGUSR1,SIG_IGN);    //Set up the threads to ignore SIGUSR1
    threadPool = malloc(numberOfThreads * sizeof(pthread_t));
    for(int i=0; i<numberOfThreads; i++){
        threadPool[i] = createThread(worker, i);
    }
    signal(SIGUSR1,handler);    //Assign 'handler' as the signal handler for SIGUSR1

    //Pollfd struct for the command port
    struct pollfd * commandPortFD = malloc(sizeof(struct pollfd));
    commandSocket = createSocket();
    listenForConnections(commandSocket, commandPort);
    commandPortFD->fd = commandSocket;
    commandPortFD->events = POLLIN;
    commandPortFD->revents = 0;

    //Create the save directory
    createDirectory(saveDir);

    printf("\nCrawling is in progress...\n");

    //While crawling is underway, poll the command port
    while(!done)
        if(poll(commandPortFD,1,1000) != 1) { if(done) break; }
        else{
            if(verbose) printf("Informing the command port that we are not done crawling\n");
            acceptConnectionWhileCrawling(commandSocket);
            commandPortFD->revents = 0;
        }
    printf("Crawling has finished.\n");
    printf("Preparing for indexing.\n");

    //Join threads
    for(int i=0; i<numberOfThreads; i++){
        pthread_cond_broadcast(&cond_nonempty);
        joinThread(threadPool[i]);
    }

    free(threadPool);

    if(verbose) printf("Queue is:\n");
    if(verbose) printQueue(queue);

    unlink("index.txt");    //Delete index.txt if it already exists
    FILE *stream = fopen("index.txt", "ab+");

    //Create a queue with the names of the folders that contain pages
    //ex. site0/ site1/ etc.
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
    free(host);
    free(saveDir);

    printf("Indexing in progress...\n");

    //Load jobExecutor/workers to be able to search the files
    jobExecutor("index.txt",4);

    printf("Indexing has finished. Crawler ready to receive SEARCH commands...\n");

    //Poll command port
    done = 0;
    commandPortFD->revents = 0;
    while(!done)
        if(poll(commandPortFD,1,1000) != 1) { if(done) break; }
        else{
            acceptCommandConnection(commandSocket);
            commandPortFD->revents = 0;
        }

    //Clean up jobExecutor and exit
    cleanUp();
    free(commandPortFD);
    unlink("index.txt");
}                     /* Close socket and exit */
