#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <string.h>
#include <sys/stat.h>

typedef struct Node2
{
    char *command;
    int pid;
    struct Node2 *next;
} Node2;

typedef struct Queue2
{
    Node2 *front, *rear;
} Queue2;

Node2 *newNode2(char *command, int pid)
{
    Node2 *temp = (Node2 *)malloc(sizeof(Node2));
    temp->command = command;
    temp->pid = pid;
    temp->next = NULL;
    return temp;
}

Queue2 *createQueue2()
{
    Queue2 *q = (Queue2 *)malloc(sizeof(Queue2));
    q->front = q->rear = NULL;
    return q;
}

void enQueue2(Queue2 *q, char *command, int pid)
{
    Node2 *temp = newNode2(command, pid);
    if (q->rear == NULL)
    {
        q->front = q->rear = temp;
        return;
    }
    q->rear->next = temp;
    q->rear = temp;
}

Node2 *deQueue2(Queue2 *q)
{
    if (q->front == NULL)
    {
        return NULL;
    }
    Node2 *temp = q->front;
    q->front = q->front->next;
    if (q->front == NULL)
        q->rear = NULL;
    return temp;
}