#include "dirInfo.h"

typedef struct SearchInfo{
    int line;                   //Number of line in which a word was found
    fileInfo * file;            //Pointer to the file the word was found
    struct SearchInfo* next;    //Next element of the list
} SearchInfo;


//List used to calculate the number of unique
//terms that where found while searching
typedef struct SearchTermList{
    char * term;                    //String of term
    struct SearchTermList* next;    //Next element of the list
} SearchTermList;


SearchInfo * newSearchInfo(int line, fileInfo * file);
void freeSearchInfo(SearchInfo * list);
int addSearchResult(int line, fileInfo * file, SearchInfo ** list);
void printSearchResults(SearchInfo * list);
char * searchInfoToString(SearchInfo * list);
int getNumberOfDigits(int i);

SearchTermList * newSearchTermList(char * term);
void freeSearchTermList(SearchTermList * list);
void addSearchTermList(char * term, SearchTermList ** list);
int getSearchTermListLength(SearchTermList * list);
