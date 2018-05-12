#include "threads.h"

int createSocket();
void listenForConnections(int sock, int port);
void acceptConnection(int sock);
void * serveClientThread(void * argp);
void serveClient(int sock);
char * getFullAddress(char * relativeAddress);
char * createResponse(char * content);
int countBytes(char * file);
char * getContent(char * file);
