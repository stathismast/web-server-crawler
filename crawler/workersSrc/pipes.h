#include <sys/types.h>
#include <sys/wait.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <time.h>

#define MSGSIZE 511

void openPipes();
void readFromPipe(char * msgbuf);
void writeToPipe(char * msgbuf);
