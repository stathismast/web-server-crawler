#include <sys/types.h>                                   /* For sockets */
#include <sys/socket.h>                                  /* For sockets */
#include <netinet/in.h>                         /* For Internet sockets */
#include <netdb.h>                                 /* For gethostbyaddr */
#include <stdio.h>                                           /* For I/O */
#include <stdlib.h>                                         /* For exit */
#include <string.h>                                /* For strlen, bzero */
#include <unistd.h>

void listenForConnections(int sock, int port);
char * getFullAddress(char * relativeAddress);
char * createResponse(char * content);
int countBytes(char * file);
char * getContent(char * file);
