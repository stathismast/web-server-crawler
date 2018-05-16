#include "threads.h"
#include "queue.h"

int createSocket();
void listenForConnections(int sock, int port);
void acceptConnection(int sock);
void * worker(void * argp);
void serveRequest(int sock);
char * getFullAddress(char * relativeAddress);
char * createResponse(char * content);
int countBytes(char * file);
char * getContent(char * file);

void place(Queue ** queue, int data);
int obtain(Queue ** queue);
void * worker(void * argp);
