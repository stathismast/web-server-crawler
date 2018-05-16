#include "queue.h"

//Create and initialize a new Queue
Queue * newQueue(int fd){
    // printf("Creating a new queue\n");
    Queue * head = malloc(sizeof(Queue));
    head->first = malloc(sizeof(QNode));
    head->first->fd = fd;
    head->first->next = NULL;
    head->last = head->first;
    head->length = 1;
    return head;
}

void initQueue(Queue ** queue){
    *queue = malloc(sizeof(Queue));
    (*queue)->first = NULL;
    (*queue)->last = NULL;
    (*queue)->length = 0;
}

//Deallocate spacd of all the nodes of a Queue
void freeQueue(Queue * queue){
    if(queue == NULL) return;
    freeQNode(queue->first);
    free(queue);
}

//Deallocate space used of a QNode and all its subsequent nodes
void freeQNode(QNode * node){
    if(node == NULL) return;
    freeQNode(node->next);
    free(node);
}

//Add the given letter at the end of a queue
void addToQueue(int fd, Queue ** queue){
    printf("Adding %d: ",fd);
    if(*queue == NULL) *queue = newQueue(fd);
    else{
        if((*queue)->last != NULL){
            (*queue)->last->next = malloc(sizeof(QNode));
            (*queue)->last->next->fd = fd;
            (*queue)->last->next->next = NULL;
            (*queue)->last = (*queue)->last->next;
        }
        else{
            (*queue)->first = malloc(sizeof(QNode));
            (*queue)->first->fd = fd;
            (*queue)->first->next = NULL;
            (*queue)->last = (*queue)->first;
        }
        (*queue)->length++;
    }
}

//Pop the first node from the queue
int popFromQueue(Queue ** queue){
    if(*queue == NULL) return -1;
    if((*queue)->first == NULL) return -1;

    int fd;
    if((*queue)->first == (*queue)->last){
        fd = (*queue)->first->fd;
        free((*queue)->first);
        (*queue)->first = NULL;
        (*queue)->last = NULL;
    }
    else{
        fd = (*queue)->first->fd;
        QNode * newFirst = (*queue)->first->next;
        free((*queue)->first);
        (*queue)->first = newFirst;
    }
    (*queue)->length--;
    return fd;
}

void printQueue(Queue * queue){
    if(queue == NULL) { printf("Len: %d ", queue->length); return; }
    if(queue->first == NULL) { printf("Len: %d ", queue->length); return; }

    printf("Len: %d:\t", queue->length);
    QNode * node = queue->first;
    while(node != NULL){
        printf("%d ", node->fd);
        node = node->next;
    }
    printf("\n");
}
