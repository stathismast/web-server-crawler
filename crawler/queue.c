#include "queue.h"

Queue * newQueue(char * fileName){
    Queue * node = malloc(sizeof(Queue));
    node->fileName = malloc(strlen(fileName)+1);
    strcpy(node->fileName,fileName);
    node->next = NULL;
    return node;
}

void freeQueue(Queue * queue){
    if(queue == NULL) return;
    freeQueue(queue->next);
    free(queue->fileName);
    free(queue);
}

void addToQueue(char * fileName, Queue ** queue){
    if(*queue == NULL){
        (*queue) = newQueue(fileName);
        return;
    }

    if(strcmp(fileName,(*queue)->fileName) == 0)
        return;

    return addToQueue(fileName,&(*queue)->next);
}

int getQueueLength(Queue * queue){
    if(queue == NULL) return 0;
    return 1 + getQueueLength(queue->next);
}

void printQueue(Queue * queue){
    if(queue == NULL) return;
    printf("%s\n",queue->fileName);
    printQueue(queue->next);
}
