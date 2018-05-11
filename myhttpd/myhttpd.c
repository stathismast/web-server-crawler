#include "server.h"

char response[1024] = "HTTP/1.1 200 OK\nDate: Mon, 27 May 2018 12:28:53 GMT\nServer: myhttpd/1.0.0 (Ubuntu64)\nContent-Length: 38\nContent-Type: text/html\nConnection: Closed\n\n<html>hello one two three four</html>";
char errorMsg[1024] = "HTTP/1.1 403 OK\nDate: Mon, 27 May 2018 12:28:53 GMT\nServer: myhttpd/1.0.0 (Ubuntu64)\nContent-Length: 46\nContent-Type: text/html\nConnection: Closed\n\n<html>404 Error: Cannot find this file</html>";
char notGet[1024] = "HTTP/1.1 403 OK\nDate: Mon, 27 May 2018 12:28:53 GMT\nServer: myhttpd/1.0.0 (Ubuntu64)\nContent-Length: 57\nContent-Type: text/html\nConnection: Closed\n\n<html>Error: This server only support GET requests</html>";

char rootDir[50] = "/mnt/d/DI/linux/home/syspro/third/script/root_dir";

struct sockaddr_in server;

int main(int argc, char *argv[]){
    int sock;
    int newsock;
    int port;
    char buf[4096];
    struct sockaddr_in client;
    struct sockaddr * clientptr;
    unsigned int clientlen;
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
    listenForConnections(sock, port);

    while(1){
        //Accept connection
        clientlen = sizeof client;
        if ((newsock = accept(sock, (struct sockaddr *) &client, &clientlen)) < 0){
            perror("accept"); exit(1);
        }

        //Find clients address
        if ((rem = gethostbyaddr((char *) &client.sin_addr.s_addr,
                                          sizeof client.sin_addr.s_addr,
                                          client.sin_family)) == NULL) {
            perror("gethostbyaddr"); exit(1);
        }
        printf("Accepted connection from %s\n", rem->h_name);

        //Create child to serve the client
        switch(fork()){
            case -1:
                perror("fork"); exit(1);
            case 0:     //Child process
                bzero(buf, sizeof buf); //Init buffer
                if (read(newsock, buf, sizeof buf) < 0){ //Get message
                    perror("read"); exit(1);
                }

                //Check that we received a GET command
                strtok(buf," ");
                if(strcmp(buf,"GET") != 0 ){
                    if (write(newsock, notGet, sizeof notGet) < 0){
                        perror("write"); exit(1);
                    }
                    exit(0);
                }

                //Store requested file name
                char * relativeAddress = strtok(NULL," ");
                if(relativeAddress == NULL){
                    if (write(newsock, errorMsg, sizeof errorMsg) < 0){//Send message
                        perror("write"); exit(1);
                    }
                    exit(0);
                }

                char * htmlFile = getFullAddress(relativeAddress);
                printf("File requested: %s\n", htmlFile);

                char * content = getContent(htmlFile);
                if(content == NULL){
                    if (write(newsock, errorMsg, sizeof errorMsg) < 0){//Send message
                        perror("write"); exit(1);
                    }
                    exit(0);
                }

                char * htmlResponse = createResponse(content);
                // printf("\n%s\n",htmlResponse);
                if (write(newsock, htmlResponse, (int)strlen(htmlResponse)+1) < 0){//Send message
                    perror("write"); exit(1);
                }

                close(newsock);           //Close socket
                exit(0);
        }
    }
}
