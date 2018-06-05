#include "ioManager.h"

extern int w;
extern struct workerInfo * workers;
extern int numberOfDirectories;
extern char ** allDirectories;

char msgbuf[MSGSIZE+1];

//Removes a new line character at the end of the string (if it exists)
void removeNewLine(char ** str){
    for(int i=0; i<strlen(*str); i++){
        if((*str)[i] == '\n'){
            (*str)[i] = 0;
        }
    }
}

//Counts the total number of lines in the given file
int countLines(char * file){
    FILE *stream;
    char *line = NULL;
    size_t len = 0;

    if((stream = fopen(file, "r")) == NULL){
        printf("Cannot open given docfile.");
        exit(4);
    }

    int lineCounter = 0;
    while(getline(&line, &len, stream) != -1) {
        lineCounter++;
    }

    free(line);
    fclose(stream);
    return lineCounter;
}

//Stores the lines of a given files in an array of string (directory)
int getLines(char * file, char *** directories){
    int lineCounter = countLines(file);
    *directories = malloc(lineCounter*sizeof(char*));

    FILE *stream;
    char *line = NULL;
    size_t len = 0;

    if((stream = fopen(file, "r")) == NULL){
        printf("Cannot open given docfile.");
        exit(4);
    }

    for(int i=0; i<lineCounter; i++){
        getline(&line, &len, stream);
        (*directories)[i] = malloc(strlen(line)+1);
        strcpy((*directories)[i],line);
        removeNewLine(&(*directories)[i]);
    }

    free(line);
    fclose(stream);
    return lineCounter;
}

//This function is in charge of deciding the
//appropriate directories for each worker
void distributeDirectories(){
    //Calculate how many directories each worker should have
    for(int i=0; i<w; i++)
        workers[i].dirCount = numberOfDirectories/w;
    for(int i=0; i<numberOfDirectories%w; i++)
        workers[i].dirCount++;

    //Store the paths of the direcectories for each worker
    int pos=0;
    for(int i=0; i<w; i++){
        workers[i].directories = malloc(workers[i].dirCount*sizeof(char*));
        for(int j=0; j<workers[i].dirCount; j++){
            workers[i].directories[j] = malloc(strlen(allDirectories[pos])+1);
            strcpy(workers[i].directories[j], allDirectories[pos]);
            pos++;
        }
    }
}


//Send directory info to workers
void sendDirectories(){
    for(int i=0; i<w; i++){
        char num[16] = {0};            //String with the number of directories
        sprintf(num, "%d", workers[i].dirCount);
        writeToChild(i,num);
        readFromPipe(i,msgbuf);
        if(strcmp(num,msgbuf) != 0){
            printf("Communication error with worker #%d.\n",i);
            exit(2);
        }
        for(int j=0; j<workers[i].dirCount; j++){
            writeToChild(i,workers[i].directories[j]);
            readFromPipe(i,msgbuf);
            if(strcmp(workers[i].directories[j],msgbuf) != 0){
                printf("Communication error with worker #%d.\n",i);
                exit(2);
            }
        }
    }
}

//Signal each worker to deallocate memory and exit
void terminateWorkers(){
    signal(SIGCHLD,SIG_DFL);    //Reset signal handler for SIGCHLD
    for(int i=0; i<w; i++){
        if(kill(workers[i].pid,0) == 0) strcpy(msgbuf,"/exit");
        if(kill(workers[i].pid,0) == 0) writeToChild(i,msgbuf);
    }
}

//Manages arguments given on execution and checks for errors
int manageArgumentsJE(int argc, char *argv[], char ** docfile, int * numWorkers){
    int gotInputFile = 0;        //Is true if '-d' argument is given
    int argumentError = 0;        //Is true if arguments are invalid
    int invalidWArgument = 0;    //Is true if '-w' argument in specific is invalid

    *numWorkers = 4;

    if(argc > 5){    //Too many arguments
        printf("ERROR: Too many arguments.\n");
        printf("Usage: ./jobExecutor -d docfile -w numWorkers\nArguments can be given in any order and only '-d' is necessary. 'numWorkers' defaults to 4.\n");
        return -1;
    }

    for(int i=1; i<argc; i++)
        //If -d is given
        if(strcmp(argv[i],"-d") == 0){
            if(argc > i+1 && !gotInputFile){
                (*docfile) = malloc((strlen(argv[i+1])+1)*sizeof(char));
                strcpy(*docfile,argv[i+1]);
                gotInputFile = 1;
                i++;
            }
            else{
                argumentError = 1;
                break;
            }
        }
        //If -w is given
        else if(strcmp(argv[i],"-w") == 0){
        if(argc > i+1){
                *numWorkers = atoi(argv[i+1]);
                if(*numWorkers < 1) {invalidWArgument = 1; break;}
                i++;
            }
            else{
                argumentError = 1;
                break;
            }
        }
        else{
            argumentError = 1;
        }

    //Print error messages
    if(invalidWArgument){
        printf("ERROR: Invalid '-w numWorkers' argument. 'numWorkers' should be a number greater than 0.\n");
        printf("Usage: ./jobExecutor -d docfile -w numWorkers\nArguments can be given in any order and only '-d' is necessary. 'numWorkers' defaults to 4.\n");
        return -1;
    }
    if(argumentError || !gotInputFile){
        printf("ERROR: Invalid arguments.\n");
        printf("Usage: ./jobExecutor -d docfile -w numWorkers\nArguments can be given in any order and only '-d' is necessary. 'numWorkers' defaults to 4.\n");
        return -1;
    }
}
