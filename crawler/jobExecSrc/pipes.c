#include "pipes.h"

extern int w;
extern struct workerInfo * workers;
extern int * out;
extern int * in;
extern char ** outPipes;
extern char ** inPipes;

int openForReading(char * name){
    return open(name, O_RDONLY);
}

int openForWriting(char * name){
    return open(name, O_WRONLY);
}

//Write to a worker AND signal the worker to check the pipe
void writeToChild(int id, char * message){
    char msgbuf[MSGSIZE+1] = {0};
    strcpy(msgbuf,message);
    if(write(out[id], msgbuf, MSGSIZE+1) == -1){
        perror("sender: error in writing:"); exit(2);
    }
    kill(workers[id].pid,SIGUSR1);    //signal child to read from pipe
}

//Write to worker WITHOUT signaling it to check the pipe
//Only used when the worker expects a message
void writeToPipe(int id, char * message){
    char msgbuf[MSGSIZE+1] = {0};
    strcpy(msgbuf,message);
    if(write(out[id], msgbuf, MSGSIZE+1) == -1){
        perror("sender: error in writing:"); exit(2);
    }
}

//Read from a pipe
void readFromPipe(int id, char * message){
    if(read(in[id], message, MSGSIZE+1) < 0){
        perror("sender: problem in reading"); exit(5);
    }
}

//Create a new named pipe and delete the old one (if it exists)
void createNamedPipe(char * pipeName){
    unlink(pipeName);
    if(mkfifo(pipeName, 0600) == -1){
        perror("sender: mkfifo");
        exit(6);
    }
}

//Allocate space for input and output pipe file descriptors and 'workers' array
void allocateSpace(){
    out = malloc(w*sizeof(int));
    in = malloc(w*sizeof(int));
    workers = malloc(w*sizeof(struct workerInfo));
}

//Open and test both input and output pipes for every worker
int openAndTestPipes(){
    char msgbuf[MSGSIZE+1] = {0};    //Buffer to be used with named pipes
    for(int i=0; i<w; i++){
        out[i]=openForWriting(outPipes[i]);
        in[i]=openForReading(inPipes[i]);
        usleep(10000);
        writeToChild(i,"/test");
        readFromPipe(i,msgbuf);
        if(strcmp(msgbuf,"/test") != 0){
            printf("Communication error with worker #%d.\n",i);
            return 0;
        }
    }
    return 1;
}

//Open input pipes with non blocking argument
void nonBlockingInputPipes(){
    for(int i=0; i<w; i++){
        close(in[i]);
        in[i] = open(inPipes[i], O_RDONLY | O_NONBLOCK);
    }
}

//Open input pipes with blocking argument
void blockingInputPipes(){
    for(int i=0; i<w; i++){
        close(in[i]);
        in[i] = open(inPipes[i], O_RDONLY);
    }
}

//Create a worker and store its information
void createReceiver(int id){
    pid_t pid = fork();
    if(pid != 0){
        workers[id].pid = pid;    //Store child pid in global array
        // printf("New child created with pid: %d\n",(int)pid);
        return;
    }
    //Create the proper arguments and exec
    char * buff[5];
    buff[0] = (char*) malloc(20);
    strcpy(buff[0],"./worker");
    buff[1] = (char*) malloc(strlen(outPipes[id])+1);
    strcpy(buff[1],outPipes[id]);
    buff[2] = (char*) malloc(strlen(inPipes[id])+1);
    strcpy(buff[2],inPipes[id]);
    char workerID[10] = {0};
    sprintf(workerID, "%d", id);
    buff[3] = (char*) malloc(strlen(workerID)+1);
    strcpy(buff[3],workerID);
    buff[4] = NULL;
    execvp("./worker", buff);
}

//Recover a terminated worker, run I/O tests and send
//information about its directories.
void reCreateReceiver(int id){
    pid_t pid = fork();
    if(pid != 0){
        workers[id].pid = pid;    //Store child pid in global array
        //Close previous pipe file descriptors
        close(in[id]);
        close(out[id]);
        //Create new I/O pipes
        createNamedPipe(outPipes[id]);
        createNamedPipe(inPipes[id]);
        //Open pipes
        out[id] = openForWriting(outPipes[id]);
        in[id] = openForReading(inPipes[id]);
        usleep(10000);
        //Run a test to verify that the pipes are running with no errors
        writeToChild(id,"/test");
        char msgbuf[MSGSIZE+1] = {0};
        readFromPipe(id,msgbuf);
        if(strcmp(msgbuf,"/test") != 0){
            printf("Communication error with worker #%d.\n",id);
            exit(2);
        }
        //Send over information about the workers directories
        char num[16] = {0};            //String with the number of directories
        sprintf(num, "%d", workers[id].dirCount);
        writeToChild(id,num);
        readFromPipe(id,msgbuf);
        if(strcmp(num,msgbuf) != 0){
            printf("Communication error with worker #%d.\n",id);
            exit(2);
        }
        for(int j=0; j<workers[id].dirCount; j++){
            writeToChild(id,workers[id].directories[j]);
            readFromPipe(id,msgbuf);
            if(strcmp(workers[id].directories[j],msgbuf) != 0){
                printf("Communication error with worker #%d.\n",id);
                exit(2);
            }
        }
        //Send a /wc command (because its part of the protocol for
        //initializing a worker before its ready to go)
        writeToChild(id,"/wc");
        readFromPipe(id,msgbuf);
        printf("Worker #%d back up and running.\n> ",id);
        return;
    }
    //Create the proper arguments and exec
    char * buff[5];
    buff[0] = (char*) malloc(20);
    strcpy(buff[0],"./worker");
    buff[1] = (char*) malloc(strlen(outPipes[id])+1);
    strcpy(buff[1],outPipes[id]);
    buff[2] = (char*) malloc(strlen(inPipes[id])+1);
    strcpy(buff[2],inPipes[id]);
    char workerID[10] = {0};
    sprintf(workerID, "%d", id);
    buff[3] = (char*) malloc(strlen(workerID)+1);
    strcpy(buff[3],workerID);
    buff[4] = NULL;
    execvp("./worker", buff);
}

//Create a name for each pipe based on the given id
void getName(int id, char ** outPipes, char ** inPipes){
    *outPipes = malloc(64);
    *inPipes = malloc(64);
    strcpy(*outPipes,"/tmp/outPipe");
    strcpy(*inPipes,"/tmp/inPipe");
    char num[10] = {0};
    sprintf(num, "%d", id+1);
    strcat(*outPipes,num);
    strcat(*inPipes,num);
}

//Create and store names for each named pipe
void getPipeNames(int pipeCount, char *** outPipes, char *** inPipes){
    *outPipes = malloc(pipeCount*sizeof(char*));
    *inPipes = malloc(pipeCount*sizeof(char*));
    for(int i=0; i<pipeCount; i++){
        getName(i,&(*outPipes)[i],&(*inPipes)[i]);
    }
}

//Deallocate space for the names of each named pipe
void freePipeNames(int pipeCount, char ** outPipes, char ** inPipes){
    for(int i=0; i<pipeCount; i++){
        free(outPipes[i]);
        free(inPipes[i]);
    }
    free(outPipes);
    free(inPipes);
}
