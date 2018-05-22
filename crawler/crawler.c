#include "queue.h"

char line[4096]; //Used to read the http header line by line

char * host;
int port;

Queue * queue;

int getNextLine(int fd){
    bzero(line, sizeof line);
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

void sendHttpRequest(char * request){
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

    //Read HTTP header and store content length
    int contentLength = 0;
    do {
        getNextLine(sock);
        if(strstr(line,"Content-Length:") != NULL){
            strtok(line," ");
            contentLength = atoi(strtok(NULL," "));
        }
    } while(strcmp(line,"\n") != 0);

    //Read file (by now we know the file's size)
    char * buffer = malloc(contentLength);
    if(read(sock, buffer, contentLength) < 0){         /* Receive message */
        perror("read"); exit(1);
    }

    //Search file for other links and add them to the queue
    char * htmlLink = buffer;
    int offset = 0;
    while((htmlLink = strstr(&htmlLink[offset],"<a href")) != NULL){
        htmlLink = strstr(htmlLink,"/");
        offset = findQuotations(htmlLink);
        char * link = malloc(offset + 1);
        strncpy(link,htmlLink,offset);
        link[offset] = 0;
        addToQueue(link,&queue);
        free(link);
    }

    free(buffer);
    close(sock);

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

int main(int argc, char *argv[]){

    manageArguments(argc,argv);

    queue = NULL;   //Initialize queue to NULL
    addToQueue("/site2/page7_16864.html",&queue);

    Queue * node = queue;
    while(node != NULL){
        char * request = createRequest(node->fileName);
        sendHttpRequest(request);
        free(request);
        node = node->next;
    }

    printQueue(queue);
    freeQueue(queue);
}                     /* Close socket and exit */
