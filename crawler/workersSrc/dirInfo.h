#include <dirent.h>
#include "trie.h"

typedef struct fileInfo{
    char * fileName;            //Name of file
    char ** lines;              //Array with all the lines
    int lineCounter;            //Number of lines
    TrieNode * trie;            //Trie for this file
} fileInfo;

typedef struct dirInfo{
    char * dirName;             //Name of directory
    int fileCount;              //Number of files in directory
    struct fileInfo * files;    //Array with information about each file
} dirInfo;

void loadDirInfo();
void printDirInfo();
void printDirContents(dirInfo * directory);
int countFiles(char * directory);
void getFiles(struct dirInfo * directory);

void removeNewLine(char ** str);
int countLines(char * file);
void getLines(fileInfo * file);
void addLinesToTrie(fileInfo * file);
int getWordCount(char * word, fileInfo * file);
int getMaxWordCount(char * word, char ** fileName);
int getMinWordCount(char * word, char ** fileName);
