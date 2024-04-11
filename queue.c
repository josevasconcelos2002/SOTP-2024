#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>
#include <fcntl.h>
#include <string.h>
#include <sys/stat.h> 

typedef struct node {
    char* request;
    struct node* next;
} node_t;

node_t* head = NULL;
node_t* tail = NULL;

pthread_mutex_t queue_mutex = PTHREAD_MUTEX_INITIALIZER;

void enqueue(char* request) {
    node_t* new_node = malloc(sizeof(node_t));
    new_node->request = strdup(request);
    new_node->next = NULL;

    pthread_mutex_lock(&queue_mutex);
    if (tail == NULL) {
        head = new_node;
        tail = new_node;
    } else {
        tail->next = new_node;
        tail = new_node;
    }
    pthread_mutex_unlock(&queue_mutex);
}

char* dequeue() {
    if (head == NULL) {
        return NULL;
    }

    pthread_mutex_lock(&queue_mutex);
    node_t* front = head;
    head = head->next;
    if (head == NULL) {
        tail = NULL;
    }
    pthread_mutex_unlock(&queue_mutex);

    char* request = strdup(front->request);
    free(front->request);
    free(front);
    return request;
}

void* read_from_pipe(void* arg) {
    int pipe = *(int*)arg;
    char buffer[128];

    // Open the queue_pipe for writing
    int queue_pipe = open("/tmp/queue_pipe", O_WRONLY);
    if (queue_pipe == -1) {
        perror("open");
        return NULL;
    }

    while (1) {
        ssize_t num_read = read(pipe, buffer, sizeof(buffer) - 1);
        if (num_read > 0) {
            buffer[num_read] = '\0';
            enqueue(buffer);

            // Write the request to queue_pipe
            write(queue_pipe, buffer, strlen(buffer));
        }
    }

    close(queue_pipe);
    return NULL;
}

int main() {
    int pipe = open("/tmp/my_pipe", O_RDONLY);
    if (pipe == -1) {
        perror("open");
        return 1;
    }

    // Create the queue_pipe
    if (mkfifo("/tmp/queue_pipe", 0666) == -1) {
        perror("mkfifo");
        return 1;
    }

    pthread_t tid;
    pthread_create(&tid, NULL, read_from_pipe, &pipe);

    // Wait for the read_from_pipe thread to finish
    pthread_join(tid, NULL);

    return 0;
}