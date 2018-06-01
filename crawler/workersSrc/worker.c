#include "commands.h"

int in;                     //Input pipe file descriptor
int out;                    //Output pipe file descriptor
char * inPipe;              //Input pipe name
char * outPipe;             //Output pipe name
char * id;                  //Worker id

int dirCount;               //Number of directories
dirInfo * directories;      //Array with information about each directory

int totalLines;             //Total number of lines for /wc
int totalWords;             //Total number of words for /wc
int totalLetters;           //Total number of letters for /wc

int stage;                  //Stage indicator, used to differentiate the stages
                            //of the workers' initialization
int done;                   //Integer ised as a boolean, has true value when
                            //the worker has to terminate

SearchTermList * stList;    //List for every unique search term found
SearchInfo * searchResults; //List of search results
int deadline;               //Integer used as a boolean

int commandID;              //Integer indicating which command we need to run

FILE * myLog;               //Log file

//Function to manage given argc & argv arguments
void manageArguments(int argc, char *argv[]){
    inPipe = malloc(strlen(argv[1])+1);
    strcpy(inPipe,argv[1]);
    outPipe = malloc(strlen(argv[2])+1);
    strcpy(outPipe,argv[2]);
    id = malloc(strlen(argv[3])+1);
    strcpy(id,argv[3]);
}

int main(int argc, char *argv[]){
    setupSigActions();

    manageArguments(argc,argv);

    //Open input and output pipes
    openPipes();

    //Initialize global variable values
    totalLines = 0;
    totalWords = 0;
    totalLetters = 0;
    deadline = 0;
    done = 0;
    stage = 1;
    commandID = 0;
    stList = NULL;

    //Create and open a log file
    char * logFileName = malloc(strlen(id)+16);
    sprintf(logFileName,"log/Worker_%s.log",id);
    myLog = fopen(logFileName, "w");

    //Pause until we get a signal from the job executor
    //Then depending in the command ID run the required command
    while(!done){
        pause();
        if(commandID == 1) maxcount();
        else if(commandID == 2) mincount();
        else if(commandID == 3) search();
        commandID = 0;
    }

    //When we're done, close log file and deallocate all the used memory
    fclose(myLog);
    free(logFileName);
    free(inPipe);
    free(outPipe);
    free(id);
    for(int i=0; i<dirCount; i++){
        for(int j=0; j<directories[i].fileCount; j++){
            free(directories[i].files[j].fileName);
            for(int k=0; k<directories[i].files[j].lineCounter; k++){
                free(directories[i].files[j].lines[k]);
            }
            free(directories[i].files[j].lines);
            freeTrie(directories[i].files[j].trie);
        }
        free(directories[i].dirName);
        free(directories[i].files);
    }
    free(directories);

    //Return the number of unique terms found
    int uniqueTerms = getSearchTermListLength(stList);
    freeSearchTermList(stList);
    return uniqueTerms;
}
