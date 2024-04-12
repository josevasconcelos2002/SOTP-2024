#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <string.h>
#include <sys/stat.h> 

typedef struct Node {
    char* command;
    int pid;
    struct Node* next;
} Node;

typedef struct Queue {
    Node *front, *rear;
} Queue;

Node* newNode(char* command, int pid) {
    Node* temp = (Node*)malloc(sizeof(Node));
    temp->command = command;
    temp->pid = pid;
    temp->next = NULL;
    return temp;
}

Queue* createQueue() {
    Queue* q = (Queue*)malloc(sizeof(Queue));
    q->front = q->rear = NULL;
    return q;
}

void enQueue(Queue* q, char* command, int pid) {
    Node* temp = newNode(command, pid);
    if (q->rear == NULL) {
        q->front = q->rear = temp;
        return;
    }
    q->rear->next = temp;
    q->rear = temp;
}

Node* deQueue(Queue* q) {
    if (q->front == NULL) {
        return NULL;
    }
    Node* temp = q->front;
    q->front = q->front->next;
    if (q->front == NULL)
        q->rear = NULL;
    return temp;
}
