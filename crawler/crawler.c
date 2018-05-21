#include "queue.h"

char line[4096]; //Used to read the http header line by line

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

void sendHttpRequest(char * request, int argc, char *argv[]){
    int port, sock; unsigned int serverlen;
       struct sockaddr_in server;
       struct sockaddr *serverptr;
       struct hostent *rem;
       if (argc < 3) {     /* Are server's host name and port number given? */
          printf("Please give host name and port number\n"); exit(1); }
       if ((sock = socket(PF_INET, SOCK_STREAM, 0)) < 0) { /* Create socket */
          perror("socket"); exit(1); }
       if ((rem = gethostbyname(argv[1])) == NULL) { /* Find server address */
          perror("gethostbyname"); exit(1); }
       port = atoi(argv[2]);              /* Convert port number to integer */
       server.sin_family = PF_INET;                      /* Internet domain */
       bcopy((char *) rem -> h_addr, (char *) &server.sin_addr,
             rem -> h_length);
       server.sin_port = htons(port); /* Server's Internet address and port */
       serverptr = (struct sockaddr *) &server;
       serverlen = sizeof server;
       if (connect(sock, serverptr, serverlen) < 0) { /* Request connection */
          perror("connect"); exit(1); }
       // printf("Requested connection to host %s port %d\n", argv[1], port);



          // bzero(buf, sizeof buf);                      /* Initialize buffer */
          // printf("Give input string: ");
          // fgets(buf, sizeof buf, stdin);         /* Read message from stdin */
          // strcpy(buf, request);
          // buf[strlen(buf)-1] = '\0';            /* Remove newline character */
          if (write(sock, request, strlen(request)) < 0) {           /* Send message */
             perror("write"); exit(1); }

          ssize_t bytesRead;

          int contentLength = 0;
          do {
            getNextLine(sock);
            // printf("line: -%s-\n", line);
            if(strstr(line,"Content-Length:") != NULL){
                strtok(line," ");
                contentLength = atoi(strtok(NULL," "));
                // printf("CONTENT LENGTH IS %d\n", contentLength);
            }
        } while(strcmp(line,"\n") != 0);

          char * buffer = malloc(contentLength);
          if (bytesRead = read(sock, buffer, contentLength) < 0) {         /* Receive message */
             perror("read"); exit(1); }
          // printf("Read string:\n%s", buffer);

          char * htmlLink = buffer;
          int offset = 0;
          while((htmlLink = strstr(&htmlLink[offset],"<a ")) != NULL){
              htmlLink = strstr(htmlLink,"/");
              offset = findQuotations(htmlLink);
              char * link = malloc(offset + 1);
              strncpy(link,htmlLink,offset);
              link[offset] = 0;
              // printf("-%s-\n", link);
              addToQueue(link,&queue);
              free(link);
          }

          // printf("\nBytes read: %d\n",(int)bytesRead);

          free(buffer);
       close(sock);

}

int main(int argc, char *argv[]){
    queue = NULL;
    addToQueue("/site2/page7_16864.html",&queue);

    Queue * node = queue;
    while(node != NULL){
        char * request = malloc(strlen(node->fileName) + 5);
        bzero(request, strlen(node->fileName) + 5);
        strcpy(request,"GET ");
        strcat(request,node->fileName);
        sendHttpRequest(request,argc,argv);
        free(request);
        node = node->next;
    }

    printQueue(queue);
    freeQueue(queue);
}                     /* Close socket and exit */
