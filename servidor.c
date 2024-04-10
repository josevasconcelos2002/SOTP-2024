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
}

void printProgramStatusesPipe(int pipe) {
    char buffer[256];
    for (int i = 0; i < programCount; i++) {
        ProgramStatus *ps = &programStatuses[i];
        snprintf(buffer, sizeof(buffer), "Program ID: %d\nCommand: %s\nStatus: %s\nTime: %ld ms\n\n", 
                 ps->id, ps->command, ps->status, ps->time);
        write(pipe, buffer, strlen(buffer));
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

int mysystem (const char* command) {

    int res = -1;
    pid_t pid = fork();
    if(pid == 0){
        char *argm[128];
        int i = 0;

        // Copy the command to a non-const string
        char command_copy[128];
        strncpy(command_copy, command, sizeof(command_copy));
        command_copy[sizeof(command_copy) - 1] = '\0'; // Ensure null-termination

        char *token = strtok(command_copy," ");
        while(token != NULL){
            argm[i] = token;
            //printf("%s\n",token);
            token = strtok(NULL, " ");
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
            return res;
        }
    }

}



int main() {
    char buffer[128];
    unlink("/tmp/my_pipe");
    unlink("/tmp/my_pipe_out");



    // Create the pipe
    if (mkfifo("/tmp/my_pipe", 0666) == -1) {
        perror("mkfifo");
        return 1;
    }

    int pipe = open("/tmp/my_pipe", O_RDONLY | O_NONBLOCK);
    if (pipe == -1) {
        perror("open");
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

    while (1) {
        ssize_t num_read = read(pipe, buffer, sizeof(buffer) - 1);
        if(num_read > 0) {
            buffer[num_read] = '\0'; // null terminate the string
            printf("Received: %s\n", buffer);

            if(strcmp(buffer, "status") == 0){
                
                printProgramStatusesPipe(pipe_out);
                continue;
            }

            int currentProgramId = programCount;
            updateProgramStatus(currentProgramId,buffer, "Executing",0);

            struct timespec start, end;

            clock_gettime(CLOCK_MONOTONIC, &start);

            pid_t pid = fork();
            if (pid == -1) {
                perror("fork");
                return 1;
            }

            if (pid == 0) {
                // Child process
                int status = mysystem2(buffer);

                if (status == -1) {
                    perror("mysystem");
                    return 1;
                }
                _exit(EXIT_SUCCESS);
            } else {
                // Parent process
                int status;
                waitpid(pid, &status, 0);
                clock_gettime(CLOCK_MONOTONIC, &end);
                long elapsed_time = (end.tv_sec - start.tv_sec) * 1000 + (end.tv_nsec - start.tv_nsec) / 1000000;
                if (WIFEXITED(status)) {
                    printf("Child exited with status %d\n", WEXITSTATUS(status));
                    if(WEXITSTATUS(status) == 0){
                        updateProgramStatus(currentProgramId,buffer, "Completed", elapsed_time);
                    }else{
                        updateProgramStatus(currentProgramId,buffer, "Error", elapsed_time);
                    }
                } else if (WIFSIGNALED(status)) {
                    printf("Child killed by signal %d\n", WTERMSIG(status));
                    updateProgramStatus(currentProgramId,buffer, "Error", elapsed_time);
                }
            }
            programCount++;
        }
        sleep(1); // wait for 1 second
    }

    close(pipe);
    close(pipe_out);

    return 0;
}