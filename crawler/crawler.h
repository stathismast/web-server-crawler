#include "queue.h"
#include "threads.h"
#include "jobExecSrc/jobExecutor.h"

int createSocket();
void listenForConnections(int sock, int port);
void acceptConnectionWhileCrawling(int sock);
void acceptCommandConnection(int sock);
void getNextLine(int fd, char * line);
int findQuotations(char * str);
int sendHttpRequest(char * request);
char * readHttpResponse(int sock);
Queue * findLinks(char * content);
void manageArguments(int argc, char *argv[]);
char * createRequest(char * fileName);
void getDirectoryAndFileNames(char * request, char ** dir, char ** fileName);
void createDirectory(char * directory);
void writeFile(char * fileName, char * content);
void * worker(void * argp);
