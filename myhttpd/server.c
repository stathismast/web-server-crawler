#include "server.h"

extern struct sockaddr_in server;
extern char rootDir[50];

void listenForConnections(int sock, int port){
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
}

char * createResponse(char * content){
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
