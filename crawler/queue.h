#include <sys/types.h>                                   /* For sockets */
#include <sys/socket.h>                                  /* For sockets */
#include <netinet/in.h>                         /* For Internet sockets */
#include <netdb.h>                                 /* For gethostbyname */
#include <stdio.h>                                           /* For I/O */
#include <stdlib.h>                                         /* For exit */
#include <string.h>                         /* For strlen, bzero, bcopy */
#include <unistd.h>

typedef struct Queue{
    char * fileName;           //String of term
    struct Queue * next;    //Next element of the list
} Queue;

Queue * newQueue(char * term);
void freeQueue(Queue * list);
void addToQueue(char * term, Queue ** list);
int getQueueLength(Queue * list);
void printQueue(Queue * queue);
