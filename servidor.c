#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>
#include <sys/wait.h>
#include <sys/stat.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include "queue.h"


typedef struct {
    int id;
    char command[128];
    char status[128];
    int time;
} ProgramStatus;

ProgramStatus programStatuses[128];
int programCount = 0;

void updateProgramStatus(int id, const char* command, const char* status, long time) {

    ProgramStatus *ps = &programStatuses[id];
    ps->id = id + 1;
    strncpy(ps->command, command, sizeof(ps->command) - 1); // Ensure null-termination
    ps->command[sizeof(ps->command) - 1] = '\0';
    strcpy(ps->status, status);
    ps->time = time;

    printf("\n"); // Print a newline for readability
}

void printProgramStatusesPipe(int pipe) {
    char buffer[256];
    for (int i = 0; i < programCount; i++) {
        ProgramStatus *ps = &programStatuses[i];
        snprintf(buffer, sizeof(buffer), "Program ID: %d\nCommand: %s\nStatus: %s\nTime: %ld ms\n\n", 
                 ps->id, ps->command, ps->status, ps->time);
        ssize_t bytes_written = write(pipe, buffer, strlen(buffer));
        if (bytes_written < 0) {
            perror("Error writing to pipe");
            return;
        }
        if (bytes_written < strlen(buffer)) {
            fprintf(stderr, "Warning: Not all data was written to pipe\n");
        }
    }
}

void printProgramStatuses() {
    for (int i = 0; i < programCount; i++) {
        ProgramStatus *ps = &programStatuses[i];
        printf("Program ID: %d\n", ps->id);
        printf("Command: %s\n", ps->command);
        printf("Status: %s\n", ps->status);
        printf("Time: %ldms\n", ps->time);
        printf("\n"); // Print a newline for readability
    }
}


int mysystem2 (const char* command) {
    char command_copy[128];
    strncpy(command_copy, command, sizeof(command_copy));
    command_copy[sizeof(command_copy) - 1] = '\0'; // Ensure null-termination

    char *token = strtok(command_copy, "|");
    while(token != NULL){
        int res = -1;
        pid_t pid = fork();
        if(pid == 0){
            char *argm[128];
            int i = 0;

            // Copy the command to a non-const string
            char command_copy2[128];
            strncpy(command_copy2, token, sizeof(command_copy2));
            command_copy2[sizeof(command_copy2) - 1] = '\0'; // Ensure null-termination

            char *token2 = strtok(command_copy2," ");
            while(token2 != NULL){
                argm[i] = token2;
                token2 = strtok(NULL, " ");
                i++;
            }
            argm[i] = NULL;
            execvp(argm[0],argm); // Pass argm[0] instead of command
            _exit(EXIT_FAILURE);
        }else if (pid < 0){
            perror("Error no fork");
            exit(EXIT_FAILURE);
        }else {
            int status = 0;
            int wait_pid = wait(&status);
            if(WIFEXITED(status)){
                res = WEXITSTATUS(status);
                if(res != 0) return res;
            }
        }
        token = strtok(NULL, "|");
    }
    return 0;
}




int main() {
    char buffer[4000];

    unlink("/tmp/my_pipe_out");
    unlink("/tmp/my_pipe");


    if(mkfifo("/tmp/my_pipe", 0666) == -1){
        perror("mkfifo");
        return 1;
    }

    if (mkfifo("/tmp/my_pipe_out", 0666) == -1) {
        perror("mkfifo");
        return 1;
    }

    int pipe_out = open("/tmp/my_pipe_out", O_WRONLY);
    if (pipe_out == -1) {
        perror("open");
        return 1;
    }

    int pipe = open("/tmp/my_pipe", O_RDONLY);
    if (pipe == -1) {
        perror("open");
        return 1;
    }

    Queue* q = createQueue();

    while (1) {
        int length; 
        ssize_t num_read = read(pipe, &length, sizeof(length));
        if(num_read > 0) {
            char buffer[556];
            num_read = read(pipe,buffer, length);
            if(num_read > 0){
                buffer[num_read] = '\0';
                char *pid_command = strtok(buffer," ");
                char *time = strtok(NULL," ");                     

                if (pid_command != NULL && time != NULL) {
                    char* command = time + strlen(time) + 1;

                    printf("Received PID: %s\n", pid_command);
                    int pid_command2 = atoi(pid_command);
                    printf("Received Time: %s\n", time);
                    long time2 = atol(time);
                    printf("Received Command: %s\n", command);

                    enQueue(q, command, pid_command2); 
                }
            }
        }


        Node* node = deQueue(q); // Get the next command from the queue
        
        
        if (node != NULL) {
            pid_t pid = fork();
            int currentProgramId = programCount;
            programCount++;
            if (pid == -1) {
                perror("fork");
                return 1;
            }

            if (pid == 0) {
                // Child process
                char* command = node->command;
                int pid2 = node->pid;
                updateProgramStatus(currentProgramId,command, "Executing",0);


                if(strcmp(command, "status") == 0){
                    char pipe_name[128];
                    snprintf(pipe_name, sizeof(pipe_name), "/tmp/my_pipe_%d", pid2);
                    unlink(pipe_name);
                    if(mkfifo(pipe_name, 0666) == -1){
                        perror("mkfifo");
                        return 1;
                    }
                    printf("pipe_name:%s.\n", pipe_name);
                    int pipe_client = open(pipe_name, O_WRONLY);
                    if(pipe_client == -1){
                        perror("pipe_client error");
                        return 1;
                    }
                    printProgramStatusesPipe(pipe_client);
                    //printProgramStatuses();

                    //printProgramStatusesPipe(pipe_out);
                    close(pipe_client);
                    _exit(EXIT_SUCCESS);
                }

                struct timespec start, end;

                clock_gettime(CLOCK_MONOTONIC, &start);

                


                int status = mysystem2(command);
                //printProgramStatuses();
                if (status == -1) {
                    perror("mysystem");
                    return 1;
                }

            
                
                clock_gettime(CLOCK_MONOTONIC, &end);
                long elapsed_time = (end.tv_sec - start.tv_sec) * 1000 + (end.tv_nsec - start.tv_nsec) / 1000000;
                if(WIFEXITED(status)){
                    printf("Child exited with status %d\n", WEXITSTATUS(status));
                    if(WEXITSTATUS(status) == 0){
                    updateProgramStatus(currentProgramId,command, "Completed", elapsed_time);
                    }else{
                        updateProgramStatus(currentProgramId,command, "Error", elapsed_time);
                    }
                }
                free(node); 
                
                _exit(EXIT_SUCCESS);
            }else{
                

            }
        }

    }
    
    close(pipe_out);
    close(pipe);

    return 0;
}
