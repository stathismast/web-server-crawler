#include "threads.h"
#include "queue.h"

int createSocket();
void listenForConnections(int sock, int port);
void acceptConnection(int sock);
void acceptCommandConnection(int sock);
char * getRunningTime();
void * worker(void * argp);
void serveRequest(int sock, int id);
char * getFullAddress(char * relativeAddress);
char * createResponse(char * content);
int countBytes(char * file);
char * getContent(char * file);

void place(Queue ** queue, int data);
int obtain(Queue ** queue, int id);
void * worker(void * argp);
