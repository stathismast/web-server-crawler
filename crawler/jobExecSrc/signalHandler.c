#include "signalHandler.h"

extern struct workerInfo * workers;
extern int w;

//Handler for when a child is terminated
void sigChild(int signum){
    int status;
    wait(&status);
    for(int i=0; i<w; i++){
        if(kill(workers[i].pid,0) != 0){
            printf("\n#%d worker terminated.\n",i);
            reCreateReceiver(i);
        }
    }
}

//Set ip handler for SIGCHLD
void setupSigActions(){
    struct sigaction sigchld;
    sigchld.sa_handler = sigChild;
    sigemptyset (&sigchld.sa_mask);
    sigchld.sa_flags = 0;
    sigaction(SIGCHLD,&sigchld,NULL);
}
