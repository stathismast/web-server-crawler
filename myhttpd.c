/* File: int_str_server.c */
#include <sys/types.h>                                   /* For sockets */
#include <sys/socket.h>                                  /* For sockets */
#include <netinet/in.h>                         /* For Internet sockets */
#include <netdb.h>                                 /* For gethostbyaddr */
#include <stdio.h>                                           /* For I/O */
#include <stdlib.h>                                         /* For exit */
#include <string.h>                                /* For strlen, bzero */
#include <unistd.h>

char response[1024] = "HTTP/1.1 200 OK\nDate: Mon, 27 May 2018 12:28:53 GMT\nServer: myhttpd/1.0.0 (Ubuntu64)\nContent-Length: 38\nContent-Type: text/html\nConnection: Closed\n\n<html>hello one two three four</html>";
char errorMsg[1024] = "HTTP/1.1 403 OK\nDate: Mon, 27 May 2018 12:28:53 GMT\nServer: myhttpd/1.0.0 (Ubuntu64)\nContent-Length: 46\nContent-Type: text/html\nConnection: Closed\n\n<html>404 Error: Cannot find this file</html>";
char notGet[1024] = "HTTP/1.1 403 OK\nDate: Mon, 27 May 2018 12:28:53 GMT\nServer: myhttpd/1.0.0 (Ubuntu64)\nContent-Length: 57\nContent-Type: text/html\nConnection: Closed\n\n<html>Error: This server only support GET requests</html>";

char rootDir[16] = "script/root_dir";

//Count the number of lines in a given file
int countBytes(char * file);
char * getContent(char * file);

int main(int argc, char *argv[]){
    int port, sock, newsock; char buf[4096];
    unsigned int serverlen, clientlen;
    struct sockaddr_in server, client;
    struct sockaddr *serverptr, *clientptr;
    struct hostent *rem;

    //Check given arguments
    if (argc < 2){
        printf("Please give the port number\n");
        exit(1);
    }

    //Create socket
    if ((sock = socket(PF_INET, SOCK_STREAM, 0)) < 0){
        perror("socket"); exit(1);
    }

    port = atoi(argv[1]);
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

    while(1){
        clientptr = (struct sockaddr *) &client;
        clientlen = sizeof client;

        //Accept connection
        if ((newsock = accept(sock, clientptr, &clientlen)) < 0){
            perror("accept"); exit(1);
        }

        //Find clients address
        if ((rem = gethostbyaddr((char *) &client.sin_addr.s_addr, sizeof client.sin_addr.s_addr, client.sin_family)) == NULL) {
            perror("gethostbyaddr"); exit(1);
        }
        printf("Accepted connection from %s\n", rem->h_name);

        //Create child to serve the client
        switch (fork()){
            case -1:    //Parent process
                perror("fork"); exit(1);
            case 0:     //Child process
                do{
                    bzero(buf, sizeof buf); //Init buffer
                    if (read(newsock, buf, sizeof buf) < 0){ //Get message
                        perror("read"); exit(1);
                    }
                    strtok(buf," ");
                    //Check that we received a GET command
                    if(strcmp(buf,"GET") != 0 ){
                        if (write(newsock, notGet, sizeof notGet) < 0){//Send message
                            perror("write"); exit(1);
                        }
                        continue;
                    }

                    //Store requested file and print it
                    char * fileRelativeAddress = strtok(NULL," ");
                    if(fileRelativeAddress == NULL) continue;
                    char * htmlFile = malloc(strlen(rootDir) + strlen(fileRelativeAddress) + 1);
                    htmlFile[0] = 0;
                    strcat(htmlFile,rootDir);
                    strcat(htmlFile,fileRelativeAddress);
                    printf("File requested: %s\n", htmlFile);
                    char * content = getContent(htmlFile);

                    if(content == NULL){
                        if (write(newsock, errorMsg, sizeof errorMsg) < 0){//Send message
                            perror("write"); exit(1);
                        }
                        continue;
                    }

                    char contentLength[1024];
                    char * htmlResponse = malloc(1024);
                    htmlResponse[0] = 0;
                    strcat(htmlResponse,"HTTP/1.1 200 OK\n");
                    strcat(htmlResponse,"Date: Mon, 27 May 2018 12:28:53 GMT\n");
                    strcat(htmlResponse,"Server: myhttpd/1.0.0 (Ubuntu64)\n");
                    sprintf(contentLength,"Content-Length: %d\n",(int)strlen(content)+1);
                    strcat(htmlResponse,contentLength);
                    strcat(htmlResponse,"Content-Type: text/html\n");
                    strcat(htmlResponse,"Connection: Closed\n\n");
                    strcat(htmlResponse,content);
                    // printf("\n%s\n",htmlResponse);
                    if (write(newsock, htmlResponse, (int)strlen(htmlResponse)+1) < 0){//Send message
                        perror("write"); exit(1);
                    }
                } while (strcmp(buf, "dne") != 0);       /* Finish on "end" */
                close(newsock);                             /* Close socket */
                exit(0);
        }
    }
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
