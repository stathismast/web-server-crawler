#include "jobExecutor.h"

int w;                          //Number of workers
int * out;                      //Array with file descriptors of output pipes
int * in;                       //Array with file descriptors of input pipes
char ** outPipes;               //Array with names of output pipes
char ** inPipes;                //Array with names of input pipes
struct workerInfo * workers;    //Array of structs with info about each worker
int numberOfDirectories;        //Total number of directories
char ** allDirectories;         //Array with the names of each directory

int totalLines;                 //Total number of lines for /wc command
int totalWords;                 //Total number of words for /wc command
int totalLetters;               //Total number of lettter for /wc command

int responses;                  //Number of responses from workers
                                //Used while waiting for search results
int searching;                  //Integer used as a boolean. Has a 'true' value
                                //when we are executing a /search command
int deadline;                   //Deadline in seconds

void jobExecutor(char * docfile, int numberOfWorkers){
    w = numberOfWorkers;

    setupSigActions();

    //Initialize global variables
    totalLines = 0;
    totalWords = 0;
    totalLetters = 0;
    searching = 0;

    //Read and check arguments for validity

    //Determine whether we need to reduce the number of worker because there
    //will be more workers than directories
    numberOfDirectories = getLines(docfile,&allDirectories);
    if(w>numberOfDirectories){
        w = numberOfDirectories;
        printf("The original number of workers is more than the directories in docfile. Number of workers is now set to %d.\n",w);
    }

    //Allocate space for pipe file descriptors, pipe names, array to keep
    //PIDs of workers etc.
    allocateSpace();

    //Decide which directories will be assigned to each worker
    distributeDirectories();

    //Create all the necessary named pipes
    getPipeNames(w,&outPipes,&inPipes);
    for(int i=0; i<w; i++){
        createNamedPipe(outPipes[i]);
        createNamedPipe(inPipes[i]);
    }

    //Create all the workers
    for(int i=0; i<w; i++)
        createReceiver(i);

    //Run a quick test in each pipe
    if(openAndTestPipes() == 0)
        exit(2);

    printf("Loading files...\n");
    sendDirectories();

    //Receive word count statistics from every worker
    //We do this at the start because there is no need to
    //message the worker every time we want to run the /wc command
    getWordCount();

    printf("All workers up and running.\n");

    //This is the main loop for command input and output
    // commandInputLoop();
}

void cleanUp(){
    //Inform each worker to exit
    terminateWorkers();

    //Get the workers return value
    int status;
    for(int i=0; i<w; i++){
        waitpid(workers[i].pid,&status,0);
        if (WIFEXITED(status)) {
            printf("Worker #%d found %d uniques terms.\n", i, (int)WEXITSTATUS(status));
        }
    }

    //Delete pipes
    for(int i=0; i<w; i++)
        unlink(outPipes[i]);
    for(int i=0; i<w; i++)
        unlink(inPipes[i]);

    //Deallocate all memory
    freePipeNames(w,outPipes,inPipes);
    free(in);
    free(out);
    for(int i=0; i<numberOfDirectories; i++){
        free(allDirectories[i]);
    }
    free(allDirectories);
    for(int i=0; i<w; i++){
        for(int j=0; j<workers[i].dirCount; j++){
            free(workers[i].directories[j]);
        }
        free(workers[i].directories);
    }
    free(workers);
}
//
// int main(int argc, char *argv[]){
//     char * docfile;
//     int numberOfWorkers;
//     if(manageArgumentsJE(argc,argv,&docfile,&numberOfWorkers) < 0) exit(3);
//
//     jobExecutor(docfile,numberOfWorkers);
//
//     free(docfile);
//
//     exit(0);
// }
