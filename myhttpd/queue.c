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
    printf("Adding %d:\t",fd);
    if(*queue == NULL) *queue = newQueue(fd);
    else{
        if((*queue)->last != NULL){
            // printf("Last is not NULL (Last: %d)\n", (*queue)->last->fd);
            (*queue)->last->next = malloc(sizeof(QNode));
            // printf("after malloc, last is %d and first is %d\n", (*queue)->last->fd, (*queue)->first->fd);
            (*queue)->last->next->fd = fd;
            // printf("after fd, last is %d and first is %d\n", (*queue)->last->fd, (*queue)->first->fd);
            (*queue)->last->next->next = NULL;
            // printf("after next, last is %d and first is %d\n", (*queue)->last->fd, (*queue)->first->fd);
            (*queue)->last = (*queue)->last->next;
            // printf("after its all said and done last is %d and first is %d\n", (*queue)->last->fd, (*queue)->first->fd);
        }
        else{
            // printf("Last is NULL\n");
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

    // printf("Request to pop from queue: ");
    int fd;
    if((*queue)->first == (*queue)->last){
        // printf("First is equal to last\n");
        fd = (*queue)->first->fd;
        free((*queue)->first);
        (*queue)->first = NULL;
        (*queue)->last = NULL;
    }
    else{
        // printf("First is not equal to last\n");
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
