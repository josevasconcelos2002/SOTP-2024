#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <string.h>
#include <sys/stat.h>

typedef struct Node
{
    char *command;
    int pid;
    int time;
    struct Node *next;
} Node;

typedef struct Queue
{
    Node *front, *rear;
} Queue;

Node *newNode(char *command, int pid, int time)
{
    Node *temp = (Node *)malloc(sizeof(Node));
    temp->command = command;
    temp->pid = pid;
    temp->time = time;
    temp->next = NULL;
    return temp;
}

Queue *createQueue()
{
    Queue *q = (Queue *)malloc(sizeof(Queue));
    q->front = q->rear = NULL;
    return q;
}

void enQueue(Queue *q, char *command, int pid, int time)
{ // Changed time to int
    Node *temp = newNode(command, pid, time);
    if (q->rear == NULL)
    {
        q->front = q->rear = temp;
        return;
    }

    // If the new node's time is less than the front node's time, insert at the front
    if (temp->time < q->front->time)
    { // Compare integers
        temp->next = q->front;
        q->front = temp;
    }
    else
    {
        // Else traverse the queue to find the correct position
        Node *current = q->front;
        while (current->next != NULL && current->next->time < temp->time)
        { // Compare integers
            current = current->next;
        }
        // Insert after the node that has a lesser time
        temp->next = current->next;
        current->next = temp;
        // If it was the last node, update rear
        if (current == q->rear)
        {
            q->rear = temp;
        }
    }
}

Node *deQueue(Queue *q)
{
    if (q->front == NULL)
    {
        return NULL;
    }
    Node *temp = q->front;
    q->front = q->front->next;
    if (q->front == NULL)
        q->rear = NULL;
    return temp;
}

Node *findNodeByPid(Queue *q, int pid)
{
    Node *current = q->front;
    while (current != NULL)
    {
        if (current->pid == pid)
        {
            return current;
        }
        current = current->next;
    }
    return NULL;
}

char **getCommandsInQueue(Queue *q)
{
    int count = 0;
    Node *current = q->front;
    while (current != NULL)
    {
        count++;
        current = current->next;
    }
    char **commands = (char **)malloc((count + 1) * sizeof(char *));

    current = q->front;
    for (int i = 0; i < count; i++)
    {
        commands[i] = current->command;
        current = current->next;
    }

    commands[count] = NULL;

    return commands;
}
