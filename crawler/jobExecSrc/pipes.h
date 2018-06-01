#include <sys/wait.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <time.h>

#define MSGSIZE 511

typedef struct workerInfo{
    pid_t pid;              //PID of worker
    int dirCount;           //Number of directories for this worker
    char ** directories;    //Names of directories of this worker
} workerInfo;

int openForReading(char * name);
int openForWriting(char * name);
void writeToChild(int id, char * message);
void writeToPipe(int id, char * message);
void readFromPipe(int id, char * message);
void createNamedPipe(char * pipeName);
void allocateSpace();
int openAndTestPipes();
void nonBlockingInputPipes();
void blockingInputPipes();
void createReceiver(int id);
void reCreateReceiver(int id);
void getName(int id, char ** outPipes, char ** inPipes);
void getPipeNames(int pipeCount, char *** outPipes, char *** inPipes);
void freePipeNames(int pipeCount, char ** outPipes, char ** inPipes);
