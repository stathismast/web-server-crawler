#include "crawler.h"

extern char * host;
extern int port;

extern int numberOfThreads;

extern Queue * queue;
extern Queue * nextFile;
extern pthread_mutex_t mtx;
extern pthread_cond_t cond_nonempty;
int threadsWaiting = 0;

extern int done;

void getNextLine(int fd, char * line){
    int pos = 0;
    do {
        // printf("Quarter%d: -%s-\n",pos, line);
        read(fd,&line[pos],1);
        pos++;
    } while(line[pos-1] != '\n');
}
//Return the offset to the first quotation (") character in a given string
int findQuotations(char * str){
    int offset = 0;
    while(str[offset] != '"'){
        offset++;
    }
    return offset;
}

int  sendHttpRequest(char * request){
    int sock; unsigned int serverlen;
    struct sockaddr_in server;
    struct sockaddr *serverptr;
    struct hostent *rem;

    if((sock = socket(PF_INET, SOCK_STREAM, 0)) < 0){ /* Create socket */
        perror("socket"); exit(1);
    }
    if((rem = gethostbyname(host)) == NULL){ /* Find server address */
        perror("gethostbyname"); exit(1);
    }

    server.sin_family = PF_INET;                      /* Internet domain */
    bcopy((char *) rem -> h_addr, (char *) &server.sin_addr, rem -> h_length);
    server.sin_port = htons(port); /* Server's Internet address and port */
    serverptr = (struct sockaddr *) &server;
    serverlen = sizeof server;

    if(connect(sock, serverptr, serverlen) < 0){
        perror("connect"); exit(1);
    }

    if(write(sock, request, strlen(request)) < 0){
        perror("write"); exit(1);
    }

    return sock;
}

char * readHttpResponse(int sock){
    //Read HTTP header and store content length
    int contentLength = 0;
    char line[4096];
    do {
        bzero(line, sizeof line);
        getNextLine(sock,line);
        if(strstr(line,"Content-Length:") != NULL){
            strtok(line," ");
            char * length = strtok(NULL," ");
            contentLength = atoi(length);
        }
    } while(strcmp(line,"\n") != 0);

    //Read file (by now we know the file's size)
    char * buffer = malloc(contentLength+1);
    if(read(sock, buffer, contentLength) < 0){         /* Receive message */
        perror("read"); exit(1);
    }
    buffer[contentLength] = 0;

    close(sock);

    return buffer;
}

Queue * findLinks(char * content){
    Queue * myQueue = NULL;
    char * htmlLink = content;
    int offset = 0;
    while((htmlLink = strstr(&htmlLink[offset],"<a href")) != NULL){
        htmlLink = strstr(htmlLink,"/");
        offset = findQuotations(htmlLink);
        char * link = malloc(offset + 1);
        strncpy(link,htmlLink,offset);
        link[offset] = 0;
        addToQueue(link,&myQueue);
        free(link);
    }

    return myQueue;
}

void manageArguments(int argc, char *argv[]){
    if(argc < 3){
        printf("Please give host name and port number\n"); exit(1);
    }
    host = argv[1];
    port = atoi(argv[2]);
}

char * createRequest(char * fileName){
    char * request = malloc(strlen(fileName) + 5);
    bzero(request, strlen(fileName) + 5);
    strcpy(request,"GET ");
    strcat(request,fileName);
    return request;
}

void * worker(void * argp){
    int id = (long) argp;
    while(!done){
        pthread_mutex_lock(&mtx);
            while(nextFile == NULL){
                threadsWaiting++;
                if(threadsWaiting == numberOfThreads){
                    kill(getpid(),SIGUSR1);
                    threadsWaiting--;
                    pthread_mutex_unlock(&mtx);
                    pthread_exit(0);
                }
                pthread_cond_wait(&cond_nonempty, &mtx);
                if(done){
                    threadsWaiting--;
                    pthread_mutex_unlock(&mtx);
                    pthread_exit(0);
                }
                threadsWaiting--;
            }
            char * request = createRequest(nextFile->fileName);
            nextFile = nextFile->next;
        pthread_mutex_unlock(&mtx);

        int sock = sendHttpRequest(request);
        free(request);

        char * content = readHttpResponse(sock);

        Queue * myQueue = findLinks(content);
        free(content);


        pthread_mutex_lock(&mtx);
            Queue * node = myQueue;
            while(node != NULL){
                int addedNewFile = addToQueue(node->fileName, &queue);

                //Basically, if the queue was 'empty' set nextFile to the newly added file
                if(addedNewFile && nextFile == NULL){
                    Queue * temp = queue;
                    while(temp->next != NULL) temp = temp->next;
                    nextFile = temp;
                    pthread_cond_broadcast(&cond_nonempty);
                }
                node = node->next;
            }
        pthread_mutex_unlock(&mtx);

        freeQueue(myQueue);

    }
}
