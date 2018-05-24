#include <sys/types.h>                                   /* For sockets */
#include <sys/socket.h>                                  /* For sockets */
#include <netinet/in.h>                         /* For Internet sockets */
#include <netdb.h>                                 /* For gethostbyaddr */
#include <stdio.h>                                           /* For I/O */
#include <stdlib.h>                                         /* For exit */
#include <string.h>                                /* For strlen, bzero */
#include <pthread.h>                                    /* For threads  */
#include <unistd.h>

#define perrorThreads(s,e) fprintf(stderr, "%s: %s\n", s, strerror(e))

pthread_t createThread(void *(*start_routine) (void *), int arg);
void joinThread(pthread_t thread);
