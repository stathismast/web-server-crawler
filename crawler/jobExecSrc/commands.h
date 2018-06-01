#include "signalHandler.h"

char * getCommand();
int isNumber(char * str);
void commandInputLoop();
void maxCount(char * keyword);
void minCount(char * keyword);
void search();
void getWordCount();
void printSearchResults(int worker);
void initializePollFds();
