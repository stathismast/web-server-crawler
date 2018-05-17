#include "server.h"

char response[1024] = "HTTP/1.1 200 OK\nDate: Mon, 27 May 2018 12:28:53 GMT\nServer: myhttpd/1.0.0 (Ubuntu64)\nContent-Length: 38\nContent-Type: text/html\nConnection: Closed\n\n<html>hello one two three four</html>";
char errorMsg[1024] = "HTTP/1.1 403 OK\nDate: Mon, 27 May 2018 12:28:53 GMT\nServer: myhttpd/1.0.0 (Ubuntu64)\nContent-Length: 46\nContent-Type: text/html\nConnection: Closed\n\n<html>404 Error: Cannot find this file</html>";
char notGet[1024] = "HTTP/1.1 403 OK\nDate: Mon, 27 May 2018 12:28:53 GMT\nServer: myhttpd/1.0.0 (Ubuntu64)\nContent-Length: 57\nContent-Type: text/html\nConnection: Closed\n\n<html>Error: This server only support GET requests</html>";

extern char rootDir[50];

extern pthread_mutex_t mtx;
extern pthread_cond_t cond_nonempty;
extern Queue * queue;

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
    if (listen(sock, 5) < 0){
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

    //Find clients address
    // if ((rem = gethostbyaddr((char *) &client.sin_addr.s_addr,
    //                                   sizeof client.sin_addr.s_addr,
    //                                   client.sin_family)) == NULL) {
    //     perror("gethostbyaddr"); exit(1);
    // }
    // printf("Accepted connection from %s\n", rem->h_name);

    //Safely add socket-fd in queue and signal the first available thread
    place(&queue, newsock);
    printf("producer: %d\n", newsock);
    pthread_cond_signal(&cond_nonempty);
}

void place(Queue ** queue, int data) {
    pthread_mutex_lock(&mtx);
        addToQueue(data,queue);
    pthread_mutex_unlock(&mtx);
}

int obtain(Queue ** queue) {
    int data = -1;
    pthread_mutex_lock(&mtx);

        while ((*queue)->length <= 0 && !done) {
            printf("Will be waiting \n");
            pthread_cond_wait(&cond_nonempty, &mtx);
            printf("Stopped waiting\n");
            }
        printf("About to obtain, length is %d\n", (*queue)->length);
        data = popFromQueue(queue);

    pthread_mutex_unlock(&mtx);
    return data;
}

void * worker(void * argp){
    int id = (long) argp;
    while (!done) {
        int fd = obtain(&queue);
        printf("#%ld consumer: %d\n", (long) argp, fd);

        if(fd == -1){
            printf("Value was -1\n");
            continue;
        }

        serveRequest(fd);
    }

    printf("Done has value %d\n", done);
    pthread_exit(0);
}

void serveRequest(int sock){
    char buf[4096];
    bzero(buf, sizeof buf); //Init buffer
    if (read(sock, buf, sizeof buf) < 0){ //Get message
        perror("read");
        return;
        close(sock);
    }

    //Check that we received a GET command
    strtok(buf," ");
    if(strcmp(buf,"GET") != 0 ){
        if (write(sock, notGet, sizeof notGet) < 0){
            perror("write");
            return;
            close(sock);
        }
        close(sock);
        return;
    }

    //Store requested file name
    char * relativeAddress = strtok(NULL," ");
    if(relativeAddress == NULL){
        if (write(sock, errorMsg, sizeof errorMsg) < 0){//Send message
            perror("write");
            return;
            close(sock);
        }
        close(sock);
        return;
    }

    char * htmlFile = getFullAddress(relativeAddress);
    printf("File requested: %s\n", htmlFile);

    char * content = getContent(htmlFile);
    if(content == NULL){
        if (write(sock, errorMsg, sizeof errorMsg) < 0){//Send message
            perror("write");
            return;
            close(sock);           //Close socket
            free(htmlFile);
            free(content);
        }
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
