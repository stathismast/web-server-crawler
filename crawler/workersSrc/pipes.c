#include "pipes.h"

extern int in;
extern int out;
extern char * inPipe;
extern char * outPipe;
extern char * id;

void readFromPipe(char * msgbuf){
    if(read(in, msgbuf, MSGSIZE+1) < 0){
        perror("receiver: problem in reading"); exit(5);
    }
}

void writeToPipe(char * msgbuf){
    if(write(out, msgbuf, MSGSIZE+1) == -1){
        perror("receiver: error in writing"); exit(2);
    }
}

void openPipes(){
    if((in=open(inPipe, O_RDONLY)) < 0){
        perror("receiver: inPipe open problem"); exit(3);
    }
    if((out=open(outPipe, O_WRONLY)) < 0){
        perror("receiver: outPipe open problem"); exit(3);
    }
    close(out);
    if((out=open(outPipe, O_WRONLY | O_NONBLOCK)) < 0){
        perror("receiver: outPipe open problem"); exit(3);
    }
}
