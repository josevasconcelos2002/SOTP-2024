#ifndef QUEUE_H
#define QUEUE_H

typedef struct Node {
    char* command;
    int pid;
    int time;
    struct Node* next;
} Node;

typedef struct Queue {
    Node *front, *rear;
} Queue;

Node* newNode(char* command, int pid, int time);
Queue* createQueue();
void enQueue(Queue* q, char* command, int pid, int time);
Node* deQueue(Queue* q);

#endif
