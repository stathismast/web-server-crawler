#include "server.h"

char response[1024] = "HTTP/1.1 200 OK\nDate: Mon, 27 May 2018 12:28:53 GMT\nServer: myhttpd/1.0.0 (Ubuntu64)\nContent-Length: 38\nContent-Type: text/html\nConnection: Closed\n\n<html>hello one two three four</html>";
char errorMsg[1024] = "HTTP/1.1 403 OK\nDate: Mon, 27 May 2018 12:28:53 GMT\nServer: myhttpd/1.0.0 (Ubuntu64)\nContent-Length: 46\nContent-Type: text/html\nConnection: Closed\n\n<html>404 Error: Cannot find this file</html>";
char notGet[1024] = "HTTP/1.1 403 OK\nDate: Mon, 27 May 2018 12:28:53 GMT\nServer: myhttpd/1.0.0 (Ubuntu64)\nContent-Length: 57\nContent-Type: text/html\nConnection: Closed\n\n<html>Error: This server only support GET requests</html>";

extern char * rootDir;
extern int servPort;
extern int commPort;
extern int numberOfThreads;

extern pthread_mutex_t mtx;
extern pthread_cond_t cond_nonempty;
extern Queue * queue;

unsigned long pagesServed = 0;
unsigned long bytesSent = 0;

extern int done;

extern struct timeval startingTime;

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

char * getFullAddress(char * relativeAddress){
    char * htmlFile = malloc(strlen(rootDir) + strlen(relativeAddress) + 1);
    htmlFile[0] = 0;
    strcat(htmlFile,rootDir);
    strcat(htmlFile,relativeAddress);
    return htmlFile;
}

void acceptConnection(int sock){
    int newsock;
    struct sockaddr_in client;
    unsigned int clientlen;
    struct hostent *rem;

    //Accept connection
    clientlen = sizeof client;
    if ((newsock = accept(sock, (struct sockaddr *) &client, &clientlen)) < 0){
        perror("accept"); exit(1);
    }

    //Safely add socket-fd in queue and signal the first available thread
    place(&queue, newsock);
    printf("producer: %d\n", newsock);
    pthread_cond_signal(&cond_nonempty);
}

char * getRunningTime(){
    struct timeval tv;
    char buffer[64];
    int millisec;
    struct tm* tm_info;
    gettimeofday(&tv, NULL);

    millisec = lrint((tv.tv_usec - startingTime.tv_usec)/10000.0); // Round to nearest millisec
    if (millisec>=100) { // Allow for rounding up to nearest second
        millisec -=100;
        tv.tv_sec++;
    }
    else if(millisec < 0){
        millisec += 100;
        tv.tv_sec--;
    }
    tv.tv_sec = tv.tv_sec - startingTime.tv_sec;
    tm_info = gmtime(&tv.tv_sec);

    strftime(buffer, 64, "%H:%M:%S", tm_info);
    sprintf(&buffer[strlen(buffer)],".%02d", millisec);

    char * ret = malloc(strlen(buffer)+1);
    strcpy(ret,(char *)&buffer);
    return ret;
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
    bzero(buf, sizeof buf); //Init buffer
    if (read(newsock, buf, sizeof buf) < 0){ //Get message
        perror("read");
        close(newsock);
        return;
    }

    char * temp = strtok(buf," \r\n");
    if(strcmp(temp,"SHUTDOWN") == 0){
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

    if(strcmp(temp,"STATS") == 0){
        bzero(buf,sizeof buf);

        char * runningTime = getRunningTime();

        sprintf(buf,"Server up for %s, served %li pages, %li bytes\n", runningTime, pagesServed, bytesSent);
        free(runningTime);
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

void place(Queue ** queue, int data) {
    pthread_mutex_lock(&mtx);
        addToQueue(data,queue);
    pthread_mutex_unlock(&mtx);
}

int obtain(Queue ** queue, int id) {
    int data = -1;
    pthread_mutex_lock(&mtx);

        while ((*queue)->length <= 0 && !done){
            printf("#%d Will be waiting\n", id);
            pthread_cond_wait(&cond_nonempty, &mtx);
            printf("#%d Stopped waiting\n", id);
        }

        if(done){
            pthread_mutex_unlock(&mtx);
            return -1;
        }

        // printf("#%d About to obtain, length is %d\n", id, (*queue)->length);
        data = popFromQueue(queue);

    pthread_mutex_unlock(&mtx);
    return data;
}

void * worker(void * argp){
    int id = (long) argp;
    while (!done) {
        int fd = obtain(&queue, id);
        // printf("#%ld consumer: %d\n", (long) argp, fd);

        if(fd == -1) break;

        serveRequest(fd, id);
    }

    printf("#%d Done has value %d\n", id, done);
    return 0;
}

void serveRequest(int sock, int id){
    char buf[4096];
    bzero(buf, sizeof buf); //Init buffer
    if (read(sock, buf, sizeof buf) < 0){ //Get message
        perror("read");
        close(sock);
        return;
    }

    //Check that we received a GET command
    strtok(buf," ");
    if(strcmp(buf,"GET") != 0 ){
        if (write(sock, notGet, sizeof notGet) < 0){
            perror("write");
            close(sock);
            return;
        }
        printf("#%d returning (Not a GET request)\n", id);
        close(sock);
        return;
    }

    //Store requested file name
    char * relativeAddress = strtok(NULL," ");
    if(relativeAddress == NULL){
        if (write(sock, errorMsg, sizeof errorMsg) < 0){//Send message
            perror("write");
            close(sock);
            return;
        }
        close(sock);
        printf("#%d returning (Invalid HTTP request)\n", id);
        return;
    }

    char * htmlFile = getFullAddress(relativeAddress);
    printf("#%d File requested: %s\n", id, htmlFile);

    char * content = getContent(htmlFile);
    if(content == NULL){
        if (write(sock, errorMsg, sizeof errorMsg) < 0){//Send message
            perror("write");
            close(sock);           //Close socket
            free(htmlFile);
            free(content);
            return;
        }
        printf("#%d returning (File not found)\n", id);
        close(sock);           //Close socket
        free(htmlFile);
        free(content);
        return;
    }

    char * htmlResponse = createResponse(content);
    // printf("\n%s\n",htmlResponse);
    if (write(sock, htmlResponse, (int)strlen(htmlResponse)+1) < 0){//Send message
        perror("write");
        close(sock);           //Close socket
        free(htmlFile);
        free(htmlResponse);
        free(content);
        return;
    }

    bytesSent += strlen(content);
    pagesServed++;

    close(sock);           //Close socket
    free(htmlFile);
    free(htmlResponse);
    free(content);
}

char * createResponse(char * content){
    char contentLength[1024];
    char * htmlResponse = malloc(512 + strlen(content));
    htmlResponse[0] = 0;
    strcat(htmlResponse,"HTTP/1.1 200 OK\n");
    strcat(htmlResponse,"Date: Mon, 27 May 2018 12:28:53 GMT\n");
    strcat(htmlResponse,"Server: myhttpd/1.0.0 (Ubuntu64)\n");
    sprintf(contentLength,"Content-Length: %d\n",(int)strlen(content)+1);
    strcat(htmlResponse,contentLength);
    strcat(htmlResponse,"Content-Type: text/html\n");
    strcat(htmlResponse,"Connection: Closed\n\n");
    strcat(htmlResponse,content);
    return htmlResponse;
}

//Count the number of bytes in a given file
int countBytes(char * file){
    FILE *stream;
    char *line = NULL;
    size_t len = 0;

    if((stream = fopen(file, "r")) == NULL){
        return -1;
    }

    int bytes = 0;
    while(getline(&line, &len, stream) != -1) {
        bytes += strlen(line);
    }

    free(line);
    fclose(stream);
    return bytes;
}

//Return a string with the contents of a file
char * getContent(char * file){
    int bytes = countBytes(file) + 1;
    if(bytes < 1) return NULL;
    char * content = malloc(bytes);
    content[0] = 0;

    FILE *stream;
    char *line = NULL;
    size_t len = 0;

    if((stream = fopen(file, "r")) == NULL){
        return NULL;
    }

    while(getline(&line, &len, stream) != -1){
        strcat(content,line);
    }

    free(line);
    fclose(stream);
    return content;
}

void invalidArguments(){
    printf("ERROR: Invalid arguments.\n");
    printf("Usage: ./myhttpd -p serving_port -c command_port -t num_of_threads -d root_dir\n");
    printf("Note : Arguments can be given in any order but they are all necessary.\n");
}

void manageArguments(int argc, char *argv[]){
    if(argc != 9){    //Too many arguments
        invalidArguments();
        exit(1);
    }

    int gotServingPort = 0;
    int gotCommandPort = 0;
    int gotNumOfThreads = 0;
    int gotRootDir = 0;

    int pos = 1;
    while(pos != 9){
        if(strcmp(argv[pos],"-p") == 0){
            if(gotServingPort){
                invalidArguments();
                exit(1);
            }
            gotServingPort = 1;
            servPort = atoi(argv[pos+1]);
        }
        else if(strcmp(argv[pos],"-c") == 0){
            if(gotCommandPort){
                invalidArguments();
                exit(1);
            }
            gotCommandPort = 1;
            commPort = atoi(argv[pos+1]);
        }
        else if(strcmp(argv[pos],"-t") == 0){
            if(gotNumOfThreads){
                invalidArguments();
                exit(1);
            }
            gotNumOfThreads = 1;
            numberOfThreads = atoi(argv[pos+1]);
        }
        else if(strcmp(argv[pos],"-d") == 0){
            if(gotRootDir){
                invalidArguments();
                exit(1);
            }
            gotRootDir = 1;
            rootDir = malloc(strlen(argv[pos+1])+1);
            bzero(rootDir, strlen(argv[pos+1])+1); //Initialize string
            strcpy(rootDir, argv[pos+1]);
        }
        pos+=2;
    }
    if(gotServingPort && gotCommandPort && gotRootDir && gotNumOfThreads){
        printf("Server running on port %d with %d threads.\n",servPort,numberOfThreads);
        printf("Command port is %d and root directory is %s.\n",commPort,rootDir);
    }
    else{
        invalidArguments();
        exit(1);
    }
}
