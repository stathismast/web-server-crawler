#include "server.h"

char rootDir[50] = "/mnt/d/DI/linux/home/syspro/third/script/root_dir";

int main(int argc, char *argv[]){
    int sock;
    int port;

    //Manage arguments
    if (argc < 2){
        printf("Please give the port number\n");
        exit(1);
    }
    port = atoi(argv[1]);

    sock = createSocket();
    listenForConnections(sock, port);
    while(1){
        acceptConnection(sock);
    }
}
