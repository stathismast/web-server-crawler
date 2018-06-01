#include "crawler.h"

extern char * host;
extern int port;
extern int commandPort;

extern int numberOfThreads;

extern Queue * queue;
extern Queue * nextFile;
extern pthread_mutex_t mtx;
extern pthread_cond_t cond_nonempty;
int threadsWaiting = 0;

extern char saveDir[64];

extern int done;

int createSocket(){
    int sock;
    if ((sock = socket(PF_INET, SOCK_STREAM, 0)) < 0){
        perror("socket"); exit(1);
    }
    return sock;
}

void listenForConnections(int sock, int port){
    struct sockaddr_in server;
    struct sockaddr *serverptr;
    unsigned int serverlen;

    server.sin_family = PF_INET;                      /* Internet domain */
    server.sin_addr.s_addr = htonl(INADDR_ANY);   /* My Internet address */
    server.sin_port = htons(port);                     /* The given port */
    serverptr = (struct sockaddr *) &server;
    serverlen = sizeof server;

    //Bind socket to address
    if (bind(sock, serverptr, serverlen) < 0) {
        perror("bind"); exit(1);
    }

    //Listen for connections
    if (listen(sock, 512) < 0){
        perror("listen"); exit(1);
    }
    printf("Listening for connections to port %d\n", port);
}

void acceptConnectionWhileCrawling(int sock){
    int newsock;
    struct sockaddr_in client;
    unsigned int clientlen;
    struct hostent *rem;

    //Accept connection
    clientlen = sizeof client;
    if ((newsock = accept(sock, (struct sockaddr *) &client, &clientlen)) < 0){
        perror("accept"); exit(1);
    }

    char buf[128];
    bzero(buf, sizeof buf); //Init buffer
    if (read(newsock, buf, sizeof buf) < 0){ //Get message
        perror("read");
        close(newsock);
        return;
    }

    if (write(newsock, "Crawling still in progress\n", 28) < 0){
        perror("write");
        close(newsock);
        return;
    }
    close(newsock);
    return;
}

void acceptCommandConnection(int sock){
    int newsock;
    struct sockaddr_in client;
    unsigned int clientlen;
    struct hostent *rem;

    //Accept connection
    clientlen = sizeof client;
    if ((newsock = accept(sock, (struct sockaddr *) &client, &clientlen)) < 0){
        perror("accept"); exit(1);
    }

    char buf[128];
    ssize_t bytesRead;
    bzero(buf, sizeof buf); //Init buffer
    if (bytesRead = read(newsock, buf, sizeof buf) < 0){ //Get message
        perror("read");
        close(newsock);
        return;
    }

    char temp = buf[8];
    buf[8] = 0;
    if(strcmp(buf,"SHUTDOWN") == 0){
        if (write(newsock, "Shutting down...\n", 18) < 0){
            perror("write");
            close(newsock);
            return;
        }
        printf("Command request for shutdown\n");
        close(newsock);
        done = 1;
        return;
    }
    buf[8] = temp;

    buf[6] = 0;
    if(strcmp(buf,"SEARCH") == 0){
        if (write(newsock, "Searching...\n", 14) < 0){
            perror("write");
            close(newsock);
            return;
        }
        buf[6] = ' ';

        char * saveptr;
        char * token = strtok_r(buf," \r\n",&saveptr);

        int termCount = 0;
        char * searchTerms[10];
        while(token != NULL && termCount < 10){
            if(strcmp(token,"SEARCH") == 0){
                token = strtok_r(NULL," \r\n",&saveptr);
                continue;
            }
            printf("-%s-\n",token);
            searchTerms[termCount] = malloc(strlen(token)+1);
            strcpy(searchTerms[termCount],token);
            termCount++;
            token = strtok_r(NULL," \r\n",&saveptr);
        }

        if(termCount == 0){
            if (write(newsock, "No search terms given\n", 23) < 0){
                perror("write");
                close(newsock);
                return;
            }
        }

        int storedSTDOUT = dup(STDOUT_FILENO);  //Store stdout
        dup2(newsock,STDOUT_FILENO);            //Redirect stdout to socket
        executeSearch((char **)searchTerms,termCount,5);
        dup2(storedSTDOUT,STDOUT_FILENO);       //Restore stdout

        for(int i=0; i<termCount; i++){
            free(searchTerms[i]);
        }
        close(newsock);
        return;
    }

    buf[5] = 0;
    if(strcmp(buf,"STATS") == 0){
        bzero(buf,sizeof buf);
        sprintf(buf,"Server up for X served X pages, X bytes\n");
        if (write(newsock, buf, strlen(buf)+1) < 0){
            perror("write");
            close(newsock);
            return;
        }
        printf("Command request for stats\n");
    }
    else{
        if (write(newsock, "Invalid command.\n", 18) < 0){
            perror("write");
            close(newsock);
            return;
        }
        printf("Invalid command.\n");
    }
    close(newsock);
    return;
}

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
    bzero(buffer,contentLength+1);
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
    if(argc < 4){
        printf("Please give host name and port number\n"); exit(1);
    }
    host = argv[1];
    port = atoi(argv[2]);
    commandPort = atoi(argv[3]);
}

char * createRequest(char * fileName){
    char * request = malloc(strlen(fileName) + 5);
    bzero(request, strlen(fileName) + 5);
    strcpy(request,"GET ");
    strcat(request,fileName);
    return request;
}

void getDirectoryAndFileNames(char * request, char ** dir, char ** file){
    char * directory, * fileName;
    for(int i=0; i<strlen(request); i++){
        if(request[i] == '/'){
            char temp = 0;
            int j;
            for(j=i+1; j<strlen(request); j++){
                if(request[j] == '/'){
                    temp = request[j];
                    request[j] = 0;
                    break;
                }
            }
            directory = malloc(strlen(&request[i+1])+1);
            strcpy(directory,&request[i+1]);
            request[j] = temp;
            break;
        }
    }
    for(int i=0; i<strlen(request); i++){
        if(request[i] == '/'){
            fileName = malloc(strlen(&request[i+1])+1);
            strcpy(fileName,&request[i+1]);
            break;
        }
    }

    *dir = malloc(strlen(directory) + strlen(saveDir) + 2);
    bzero(*dir,strlen(directory) + strlen(saveDir) + 2);
    strcat(*dir,saveDir);
    strcat(*dir,"/");
    strcat(*dir,directory);
    free(directory);

    *file = malloc(strlen(fileName) + strlen(saveDir) + 2);
    bzero(*file,strlen(fileName) + strlen(saveDir) + 2);
    strcat(*file,saveDir);
    strcat(*file,"/");
    strcat(*file,fileName);
    free(fileName);
}

void createDirectory(char * directory){
	struct stat st = {0};
	if (stat(directory, &st) == -1){
		mkdir(directory, 0700);
	}
}

void writeFile(char * fileName, char * content){
    unlink(fileName); //Delete file if it already exists

    // char * clean = malloc(strlen(content)+1);   //String that will contain the
    // bzero(clean,strlen(content)+1);             //content without html links
    // int htmlLink = 0;
    // for(int i=0; i<strlen(content); i++){
    //     if(content[i] == '<') htmlLink = 1;
    //     if(!htmlLink) clean[i] = content[i];
    //     if(content[i] == '>') htmlLink = 0;
    // }
    // printf("Only now ready to write\n");
    // FILE *stream = fopen(fileName, "ab+");
    // fprintf(stream,"%s",content);


    // FILE *stream = fopen(fileName, "ab+");
    // int offset = 0;
    // for(int i=0; i<strlen(content); i++){
    //     if(content[i] == '<'){
    //         char temp = content[i];
    //         content[i] = 0;
    //         if(offset != i) fprintf(stream,"%s",&content[offset]);
    //         content[i] = temp;
    //     }
    //     else if(content[i] == '>'){
    //         offset = i+1;
    //     }
    // }


    FILE *stream = fopen(fileName, "ab+");

    char * saveptr;
    char * line = strtok_r(content,"\n",&saveptr);
    while(line != NULL){
        if(line[0] != '<')
            fprintf(stream,"%s\n",line);
        line = strtok_r(NULL,"\n",&saveptr);
    }

    fclose(stream);
    // free(clean);
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

        printf("#%d Threads waiting: %d\n",id,threadsWaiting);

        int sock = sendHttpRequest(request);

        char * content = readHttpResponse(sock);

        Queue * myQueue = findLinks(content);

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

        char * directoryName;
        char * fileName;
        getDirectoryAndFileNames(request,&directoryName,&fileName);

                printf("%d About to go write file (%s)\n",id,fileName);
        // printf("%d -%s-\n",id,directoryName);
        createDirectory(directoryName);
        writeFile(fileName,content);
                printf("%d Done writing file (%s)\n",id,fileName);


        free(directoryName);
        free(fileName);
        free(request);
        free(content);
    }
}
