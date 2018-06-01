#include "signalHandler.h"

char * getCommand();
int isNumber(char * str);
void commandInputLoop();
void maxCount(char * keyword);
void minCount(char * keyword);
void search();
void executeSearch(char ** searchTerms, int termCount, int dl);
void getWordCount();
void printSearchResults(int worker);
void initializePollFds();
