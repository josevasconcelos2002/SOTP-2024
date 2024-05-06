#ifndef QUEUE2_H
#define QUEUE2_H

typedef struct Node2
{
    char *command;
    int pid;
    struct Node2 *next;
} Node2;

typedef struct Queue2
{
    Node *front, *rear;
} Queue2;

Node2 *newNode2(char *command, int pid);
Queue2 *createQueue2();
void enQueue2(Queue2 *q, char *command, int pid);
Node2 *deQueue2(Queue2 *q);

#endif