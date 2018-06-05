#include "dirInfo.h"

extern int dirCount;
extern struct dirInfo * directories;

extern int totalLines;
extern int totalWords;
extern int totalLetters;

//Load inforamtion for each directory
void loadDirInfo(){
    for(int i=0; i<dirCount; i++)
        getFiles(&directories[i]);
}

//Print out the contents of each directory. Used for debugging
void printDirInfo(){
    for(int i=0; i<dirCount; i++)
        printDirContents(&directories[i]);
}

//Print out the contents of a directory
void printDirContents(dirInfo * directory){
    printf("\t-%s-\n",directory->dirName);
    for(int i=0; i<directory->fileCount; i++){
        printf("\t\t-%s-\n", directory->files[i].fileName);
        for(int j=0; j<directory->files[i].lineCounter; j++)
            printf("\t\t\t-%s-\n", directory->files[i].lines[j]);
    }
}

//Count the number of files in the given directory
int countFiles(char * directory){
    DIR *dir;
    struct dirent *ent;
    int fileCount = 0;
    if((dir = opendir(directory)) == NULL) {
        perror ("worker");
        exit(3);
    }
      /* print all the files and directories within directory */
    while((ent = readdir(dir)) != NULL) {
        fileCount++;
    }
    closedir (dir);
    return fileCount-2; //Don't include current and parent directories
}

//Store the fileInfo for every file of a given directory
void getFiles(struct dirInfo * directory){
    DIR *dir;
    struct dirent *ent;

    directory->fileCount = countFiles(directory->dirName);
    directory->files = malloc(directory->fileCount*sizeof(fileInfo));

    if((dir = opendir(directory->dirName)) == NULL) {
        perror("worker");
        exit(3);
    }

    ent = readdir(dir); //Consume curent and parent directories
    ent = readdir(dir);
    for(int i=0; i<directory->fileCount; i++){
        ent = readdir(dir);
        directory->files[i].fileName = malloc(strlen(directory->dirName)+strlen(ent->d_name)+2);
        directory->files[i].fileName[0] = 0;
        strcat(directory->files[i].fileName,directory->dirName);
        strcat(directory->files[i].fileName,"/");
        strcat(directory->files[i].fileName,ent->d_name);
        strcat(directory->files[i].fileName,"\0");
        getLines(&directory->files[i]);
        addLinesToTrie(&directory->files[i]);
    }
    closedir (dir);
}

//Remove a new line character from the end of a string
void removeNewLine(char ** str){
    for(int i=0; i<strlen(*str); i++){
        if((*str)[i] == '\n'){
            (*str)[i] = 0;
        }
    }
}

//Count the number of lines in a given file
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

//Store the lines of a given file
void getLines(fileInfo * file){
    int lineCounter = countLines(file->fileName);
    file->lines = malloc(lineCounter*sizeof(char*));

    totalLines += lineCounter;

    FILE *stream;
    char *line = NULL;
    size_t len = 0;

    if((stream = fopen(file->fileName, "r")) == NULL){
        printf("Cannot open given docfile.");
        exit(4);
    }

    for(int i=0; i<lineCounter; i++){
        getline(&line, &len, stream);
        file->lines[i] = malloc(strlen(line)+1);
        strcpy(file->lines[i],line);
        removeNewLine(&file->lines[i]);
        totalLetters += strlen(line);
    }

    free(line);
    fclose(stream);
    file->lineCounter = lineCounter;
}

//Add the lines from a file to its trie
void addLinesToTrie(fileInfo * file){
    int wordCount;
    file->trie = NULL;
    for(int i=0; i<file->lineCounter; i++){
        wordCount = addWordsIntoTrie(file->lines[i],i,&file->trie);
        totalWords += wordCount;
    }
}

//Get the total number of times a word is in a file
int getWordCount(char * word, fileInfo * file){
    PostingListHead * pl = getPostingList(word,file->trie);
    int count;
    if(pl != NULL)
        count = pl->totalCount;
    else count = 0;
    // printf("'%s' appears in file '%s' a total of %d times.\n", word, file->fileName, count);
    return count;
}

//Get file that has the given word the most times
int getMaxWordCount(char * word,  char ** fileName){
    int maxCount = 0;
    *fileName = malloc(strlen(word)+49);
    sprintf(*fileName, "The term '%s' does not exist in the given dataset.", word);

    int count;
    for(int i=0; i<dirCount; i++)
        for(int j=0; j<directories[i].fileCount; j++){
            count = getWordCount(word,&directories[i].files[j]);
            if(count > maxCount){
                maxCount = count;
                free(*fileName);
                *fileName = malloc(strlen(directories[i].files[j].fileName)+1);
                strcpy(*fileName, directories[i].files[j].fileName);
            }
            else if(count == maxCount && count != 0){
                if(strcmp(*fileName,directories[i].files[j].fileName) > 1){
                    maxCount = count;
                    free(*fileName);
                    *fileName = malloc(strlen(directories[i].files[j].fileName)+1);
                    strcpy(*fileName, directories[i].files[j].fileName);
                }
            }
        }
        return maxCount;
}

//Get file that has the given word the least times
int getMinWordCount(char * word,  char ** fileName){
    int found = 0;
    int minCount = 0;
    *fileName = malloc(strlen(word)+49);
    sprintf(*fileName, "The term '%s' does not exist in the given dataset.", word);

    int count;
    for(int i=0; i<dirCount; i++)
        for(int j=0; j<directories[i].fileCount; j++){
            count = getWordCount(word,&directories[i].files[j]);
            if(!found && count > 0){
                minCount = count;
                free(*fileName);
                *fileName = malloc(strlen(directories[i].files[j].fileName)+1);
                strcpy(*fileName, directories[i].files[j].fileName);
                found = 1;
            }
            else if(count < minCount && count > 0){
                minCount = count;
                free(*fileName);
                *fileName = malloc(strlen(directories[i].files[j].fileName)+1);
                strcpy(*fileName, directories[i].files[j].fileName);
            }
            else if(count == minCount && count != 0){
                if(strcmp(*fileName,directories[i].files[j].fileName) > 1){
                    minCount = count;
                    free(*fileName);
                    *fileName = malloc(strlen(directories[i].files[j].fileName)+1);
                    strcpy(*fileName, directories[i].files[j].fileName);
                }
            }
        }
        return minCount;
}
