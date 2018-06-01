#include "commands.h"

extern char * id;

extern int dirCount;
extern dirInfo * directories;

extern SearchTermList * stList;
extern SearchInfo * searchResults;
extern int deadline;

extern FILE * myLog;

//Execute a maxcount command
void maxcount(){
    char msgbuf[MSGSIZE+1] = {0};

    //Read the keyword
    readFromPipe(msgbuf);
    char * word = malloc(strlen(msgbuf)+1);
    strcpy(word,msgbuf);

    //Find the file that has this keyword the most times
    char * fileName;
    int count = getMaxWordCount(word,&fileName);

    //Send the maxcount number
    sprintf(msgbuf, "%d", count);
    writeToPipe(msgbuf);

    //Send the maxcount filename
    strcpy(msgbuf,fileName);
    writeToPipe(msgbuf);

    //Log this command
    if(count != 0)
        fprintf(myLog,"%d:maxcount:%s:%s:%d\n",(int)time(NULL),word,fileName,count);
    else
        fprintf(myLog,"%d:maxcount:%s\n",(int)time(NULL),word);

    //Deallocate space
    free(word);
    free(fileName);
}

//Execute a mincount command
void mincount(){
    char msgbuf[MSGSIZE+1] = {0};

    //Read the keyword
    readFromPipe(msgbuf);
    char * word = malloc(strlen(msgbuf)+1);
    strcpy(word,msgbuf);

    //Find the file that has this keyword the least times
    char * fileName;
    int count = getMinWordCount(word,&fileName);

    //Send the mincount number
    sprintf(msgbuf, "%d", count);
    writeToPipe(msgbuf);

    //Send the mincount filename
    strcpy(msgbuf,fileName);
    writeToPipe(msgbuf);

    //Log this command
    if(count != 0)
        fprintf(myLog,"%d:mincount:%s:%s:%d\n",(int)time(NULL),word,fileName,count);
    else
        fprintf(myLog,"%d:mincount:%s\n",(int)time(NULL),word);

    //Deallocate space
    free(word);
    free(fileName);
}

//Execute a search command
void search(){
    char msgbuf[MSGSIZE+1] = {0};

    //Initialize deadline
    deadline = 0;

    //Read the number of search terms
    readFromPipe(msgbuf);
    int termCount = atoi(msgbuf);
    char ** searchTerms = malloc(termCount*sizeof(char*));

    //Read each search term and store it in the 'searchTerms' array
    for(int i=0; i<termCount; i++){
        readFromPipe(msgbuf);
        searchTerms[i] = malloc(strlen(msgbuf)+1);
        strcpy(searchTerms[i],msgbuf);
    }

    //Initialize searchResults list
    searchResults = NULL;

    //For each search term, add the files it is in to the searchResults list
    //If while we are doing this the 'deadline' variable is true, we stop
    //searching because the deadline given by the user is up
    for(int i=0; i<termCount; i++)
        if(!deadline) searchForWord(searchTerms[i]);

    //Ask the jobExecutor if we are within the deadline
    writeToPipe("deadline");
    //Read jobExecutors response
    readFromPipe(msgbuf);

    //If we are within the deadline, send the results to the jobExecutor
    if(strcmp(msgbuf,"yes") == 0)
        sendSearchResults();

    //Deallocate space
    freeSearchInfo(searchResults);
    for(int i=0; i<termCount; i++)
        free(searchTerms[i]);
    free(searchTerms);
}

//Search all the directories and all the files for the given keyword
void searchForWord(char * searchTerm){
    char msgbuf[MSGSIZE+1] = {0};

    //Initialize a string for loggin purposes
    sprintf(msgbuf,"%d:search:%s",(int)time(NULL),searchTerm);

    //For every directory and for every file, check if the search term is
    //included in the trie of that file
    for(int i=0; i<dirCount; i++)
        for(int j=0; j<directories[i].fileCount; j++){
            PostingListHead * pl = getPostingList(searchTerm,directories[i].files[j].trie);
            if(pl != NULL){
                PostingListNode * plNode = pl->next;
                //If the keyword is included in the trie, for each line its in
                //add another node to our searchResult list
                while(plNode != NULL){
                    addSearchTermList(searchTerm,&stList);
                    addSearchResult(plNode->id,&directories[i].files[j],&searchResults);
                    //Add information to the logging string
                    if(strstr(msgbuf, directories[i].files[j].fileName) == NULL){
                        strcat(msgbuf,":");
                        strcat(msgbuf,directories[i].files[j].fileName);
                    }
                    plNode = plNode->next;
                }
            }
        }
    //Log this command
    fprintf(myLog,"%s\n",msgbuf);
}

//Send the results of a search command to the jobExecutor
void sendSearchResults(){
    char msgbuf[MSGSIZE+1] = {0};
    char * temp;
    SearchInfo * node = searchResults;
    int pos = 0;
    //For every node in searchResults list
    while(node != NULL){
        //Convert a SearchInfo node to a string
        temp = searchInfoToString(node);
        //If the string is bigger that out buffer, send it over in chunks
        while((int)strlen(temp) > MSGSIZE + pos){
            strncpy(msgbuf,&temp[pos],MSGSIZE);
            msgbuf[MSGSIZE] = 0; //Null character at the end of the string
            writeToPipe(msgbuf);
            pos += MSGSIZE;
        }
        strncpy(msgbuf,&temp[pos],strlen(temp)+1);
        free(temp);
        writeToPipe(msgbuf);
        node = node->next;
    }
    strcpy(msgbuf,"noMoreResults"); //Inform the jobExecutor that we are done
    writeToPipe(msgbuf);
}
