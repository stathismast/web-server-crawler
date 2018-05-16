#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#ifndef QUEUE_H
#define QUEUE_H

typedef struct Queue{
    int length;
    struct QNode * first;    //Pointer to the first queue node
    struct QNode * last;    //Pointer to the last queue node
} Queue;

typedef struct QNode{
    int fd;                    //Integer value/File descriptor
    struct QNode * next;    //Pointer to the next queue node
} QNode;

#endif //QUEUE_H

Queue * newQueue(int fd);
void initQueue(Queue ** queue);
void freeQueue(Queue * queue);
void freeQNode(QNode * node);
void addToQueue(int fd, Queue ** queue);
int popFromQueue(Queue ** queue);
void printQueue(Queue * queue);
