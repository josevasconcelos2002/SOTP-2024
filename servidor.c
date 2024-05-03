#include <stdio.h>
#include <dirent.h>
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
#include <signal.h>
#include <sys/types.h>

#define NUM_REQUESTS 5
typedef struct
{
    int id;
    char command[300];
    char status[128];
    int time;
} ProgramStatus;

#define MAX_TASKS 100
typedef struct
{
    int id;
    char command[300];
    pid_t pid;
} Task;

Task tasks[MAX_TASKS];
int num_tasks = 0;

ProgramStatus programStatuses[128];
int programCount = 1;

void updateProgramStatus(int id, const char *command, const char *status, long time)
{

    ProgramStatus *ps = &programStatuses[id];
    ps->id = id + 1;
    strncpy(ps->command, command, sizeof(ps->command) - 1); // Ensure null-termination
    ps->command[sizeof(ps->command) - 1] = '\0';
    strcpy(ps->status, status);
    ps->time = time;

    printf("\n"); // Print a newline for readability
}
void writeProgramStatusToFile(int id, char *command, char *status, long time)
{
    char filename[128];
    snprintf(filename, sizeof(filename), "Status/program_status_%d.txt", id);

    int fd = open(filename, O_WRONLY | O_CREAT | O_APPEND, 0644); // Open the file in append mode
    if (fd == -1)
    {
        perror("Error opening file for writing");
        return;
    }

    dprintf(fd, "Program ID: %d\n", id);
    dprintf(fd, "Command: %s\n", command);
    dprintf(fd, "Status: %s\n", status);
    dprintf(fd, "Time: %ldms\n", time);
    dprintf(fd, "\n"); // Print a newline for readability

    close(fd);
}

void read_status_and_print_tasks(int pipe)
{
    DIR *dir = opendir("Status");
    if (dir == NULL)
    {
        perror("Error opening directory");
        return;
    }

    struct dirent *entry;
    while ((entry = readdir(dir)) != NULL)
    {
        if (entry->d_type == DT_REG)
        { // If the entry is a regular file
            char filename[256];
            snprintf(filename, sizeof(filename), "Status/%s", entry->d_name);

            int fd = open(filename, O_RDONLY);
            if (fd == -1)
            {
                perror("Error opening file");
                continue;
            }

            char buffer[4096];
            ssize_t bytes_read;
            while ((bytes_read = read(fd, buffer, sizeof(buffer) - 1)) > 0)
            {
                buffer[bytes_read] = '\0'; // Null-terminate the string
                ssize_t bytes_written = write(pipe, buffer, bytes_read);
                if (bytes_written < 0)
                {
                    perror("Error writing to pipe");
                    close(fd);
                    closedir(dir);
                    return;
                }
            }

            if (bytes_read == -1)
            {
                perror("Error reading file");
            }

            close(fd);
        }
    }

    closedir(dir);

    char buffer[256];
    for (int i = 0; i < num_tasks; i++)
    {
        Task *task = &tasks[i];
        snprintf(buffer, sizeof(buffer), "Program ID: %d\nCommand: %s\nStatus: Executing\n\n",
                 task->id, task->command);
        ssize_t bytes_written = write(pipe, buffer, strlen(buffer));
        if (bytes_written < 0)
        {
            perror("Error writing to pipe");
            return;
        }
        if (bytes_written < strlen(buffer))
        {
            fprintf(stderr, "Warning: Not all data was written to pipe\n");
        }
    }
}

void read_status_directory2(int pipe)
{
    DIR *dir = opendir("Status");
    if (dir == NULL)
    {
        perror("Error opening directory");
        return;
    }

    struct dirent *entry;
    while ((entry = readdir(dir)) != NULL)
    {
        if (entry->d_type == DT_REG)
        { // If the entry is a regular file
            char filename[256];
            snprintf(filename, sizeof(filename), "Status/%s", entry->d_name);

            int fd = open(filename, O_RDONLY);
            if (fd == -1)
            {
                perror("Error opening file");
                continue;
            }

            char buffer[4096];
            ssize_t bytes_read;
            while ((bytes_read = read(fd, buffer, sizeof(buffer) - 1)) > 0)
            {
                buffer[bytes_read] = '\0'; // Null-terminate the string
                ssize_t bytes_written = write(pipe, buffer, bytes_read);
                if (bytes_written < 0)
                {
                    perror("Error writing to pipe");
                    close(fd);
                    closedir(dir);
                    return;
                }
            }

            if (bytes_read == -1)
            {
                perror("Error reading file");
            }

            close(fd);
        }
    }

    closedir(dir);
}

void print_tasks_pipe(int pipe)
{
    char buffer[256];
    for (int i = 0; i < num_tasks; i++)
    {
        Task *task = &tasks[i];
        snprintf(buffer, sizeof(buffer), "Executing tasks:\n\nTask ID: %d\nCommand: %s\nPID: %d\n\n",
                 task->id, task->command, task->pid);
        ssize_t bytes_written = write(pipe, buffer, strlen(buffer));
        if (bytes_written < 0)
        {
            perror("Error writing to pipe");
            return;
        }
        if (bytes_written < strlen(buffer))
        {
            fprintf(stderr, "Warning: Not all data was written to pipe\n");
        }
    }
}
void add_task(int id, char *command, pid_t pid)
{
    // Wait until there's an open slot
    while (num_tasks >= MAX_TASKS)
    {
        sleep(1); // Sleep for 1 second
    }

    Task new_task;
    new_task.id = id;
    strncpy(new_task.command, command, sizeof(new_task.command) - 1);
    new_task.command[sizeof(new_task.command) - 1] = '\0'; // Ensure null termination
    new_task.pid = pid;

    tasks[num_tasks] = new_task;
    num_tasks++;
}

void remove_task(int id)
{
    int i;
    for (i = 0; i < num_tasks; i++)
    {
        if (tasks[i].id == id)
        {
            // Shift all tasks after this one to the left
            for (int j = i; j < num_tasks - 1; j++)
            {
                tasks[j] = tasks[j + 1];
            }
            num_tasks--;
            return;
        }
    }
}

int mysystem2(const char *command, int id)
{
    char command_copy[128];
    strncpy(command_copy, command, sizeof(command_copy));
    command_copy[sizeof(command_copy) - 1] = '\0'; // Ensure null-termination

    char *token = strtok(command_copy, "|");
    while (token != NULL)
    {
        int res = -1;
        pid_t pid = fork();
        if (pid == 0)
        {
            char *argm[128];
            int i = 0;

            // Copy the command to a non-const string
            char command_copy2[128];
            strncpy(command_copy2, token, sizeof(command_copy2));
            command_copy2[sizeof(command_copy2) - 1] = '\0'; // Ensure null-termination

            char *token2 = strtok(command_copy2, " ");
            while (token2 != NULL)
            {
                argm[i] = token2;
                token2 = strtok(NULL, " ");
                i++;
            }
            argm[i] = NULL;

            // Create a filename based on the id
            char filename[128];
            snprintf(filename, sizeof(filename), "Resultados/output_%d.txt", id);

            // Open the file in append mode
            int fd = open(filename, O_WRONLY | O_CREAT | O_APPEND, S_IRUSR | S_IWUSR);
            if (fd == -1)
            {
                perror("Error opening file for writing");
                _exit(EXIT_FAILURE);
            }

            // Redirect stdout to the file
            if (dup2(fd, STDOUT_FILENO) == -1)
            {
                perror("Error redirecting stdout to file");
                _exit(EXIT_FAILURE);
            }

            // Close the original file descriptor
            close(fd);

            execvp(argm[0], argm); // Pass argm[0] instead of command
            _exit(EXIT_FAILURE);
        }
        else if (pid < 0)
        {
            perror("Error no fork");
            exit(EXIT_FAILURE);
        }
        else
        {
            int status = 0;
            int wait_pid = wait(&status);
            if (WIFEXITED(status))
            {
                res = WEXITSTATUS(status);
                if (res != 0)
                    return res;
            }
        }
        token = strtok(NULL, "|");
    }
    return 0;
}

int main()
{
    char buffer[4000];

    unlink("/tmp/my_pipe_out");
    unlink("/tmp/my_pipe");

    if (mkdir("Resultados", 0777) == -1)
    {
        perror("Error creating directory 'Resultados'");
        return 1;
    }

    if (mkdir("Status", 0777) == -1)
    {
        perror("Error creating directory 'Status'");
        return 1;
    }

    if (mkfifo("/tmp/my_pipe", 0666) == -1)
    {
        perror("mkfifo");
        return 1;
    }

    if (mkfifo("/tmp/my_pipe_out", 0666) == -1)
    {
        perror("mkfifo");
        return 1;
    }

    int pipe_out = open("/tmp/my_pipe_out", O_WRONLY);
    if (pipe_out == -1)
    {
        perror("open");
        return 1;
    }

    int pipe = open("/tmp/my_pipe", O_RDONLY);
    if (pipe == -1)
    {
        perror("open");
        return 1;
    }

    Queue *q = createQueue();

    while (1)
    {
        int currentProgramId = programCount;
        int length;
        ssize_t num_read = read(pipe, &length, sizeof(length));
        if (num_read > 0)
        {
            char buffer[556];
            num_read = read(pipe, buffer, length);
            if (num_read > 0)
            {
                buffer[num_read] = '\0';
                char *pid_command = strtok(buffer, " ");
                char *time = strtok(NULL, " ");

                if (pid_command != NULL && time != NULL)
                {
                    char *command = time + strlen(time) + 1;

                    printf("Received PID: %s\n", pid_command);
                    int pid_command2 = atoi(pid_command);
                    printf("Received Time: %s\n", time);
                    long time2 = atol(time);
                    printf("Received Command: %s\n", command);

                    enQueue(q, command, pid_command2, time2);
                }
            }
        }

        Node *node = deQueue(q);

        if (node != NULL)
        {
            pid_t pid = fork();
            currentProgramId = programCount;
            programCount++;
            if (strcmp(node->command, "status") != 0)
            {
                add_task(currentProgramId, node->command, pid);
            }
            if (pid == -1)
            {
                perror("fork");
                return 1;
            }

            if (pid == 0)
            {
                // Child process
                char *command = node->command;
                int pid2 = node->pid;

                if (strcmp(command, "status") == 0)
                {
                    char pipe_name[128];
                    snprintf(pipe_name, sizeof(pipe_name), "/tmp/my_pipe_%d", pid2);
                    unlink(pipe_name);
                    if (mkfifo(pipe_name, 0666) == -1)
                    {
                        perror("mkfifo");
                        return 1;
                    }
                    printf("pipe_name:%s.\n", pipe_name);
                    int pipe_client = open(pipe_name, O_WRONLY);
                    if (pipe_client == -1)
                    {
                        perror("pipe_client error");
                        return 1;
                    }
                    // printProgramStatusesPipe(pipe_client);
                    // print_tasks_pipe2(pipe_client);
                    read_status_and_print_tasks(pipe_client);

                    close(pipe_client);
                    _exit(EXIT_SUCCESS);
                }

                struct timespec start, end;
                clock_gettime(CLOCK_MONOTONIC, &start);

                // writeProgramStatusToFile(currentProgramId, command, "Executing", 0);

                int status = mysystem2(command, currentProgramId);
                // printProgramStatuses();
                if (status == -1)
                {
                    perror("mysystem");
                    return 1;
                }
                clock_gettime(CLOCK_MONOTONIC, &end);

                long elapsed_time = (end.tv_sec - start.tv_sec) * 1000 + (end.tv_nsec - start.tv_nsec) / 1000000;
                if (WIFEXITED(status))
                {
                    printf("Child exited whit status %d\n", WEXITSTATUS(status));
                    if (WEXITSTATUS(status) == 0)
                    {
                        writeProgramStatusToFile(currentProgramId, command, "Completed", elapsed_time);
                    }
                    else
                    {
                        writeProgramStatusToFile(currentProgramId, command, "Error", elapsed_time);
                    }

                    _exit(EXIT_SUCCESS);
                }
            }
            else
            {
            }
            free(node);
        }
        for (int i = 0; i < num_tasks; i++)
        {
            int status;
            pid_t result = waitpid(tasks[i].pid, &status, WNOHANG);
            if (result > 0)
            {
                // Child has exited, remove the task
                remove_task(tasks[i].id);
            }
        }
    }

    close(pipe_out);
    close(pipe);

    return 0;
}
