#include <stdio.h>
#include <string.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdlib.h>

int main(int argc, char *argv[])
{
    if (argc < 2)
    {
        printf("Not Enough Arguments\n");
        return 1;
    }

    int read_pipe = open("/tmp/my_pipe_out", O_RDONLY);
    if (read_pipe == -1)
    {
        perror("open");
        return 1;
    }

    int write_pipe = open("/tmp/my_pipe", O_WRONLY);
    if (write_pipe == -1)
    {
        perror("open");
        return 1;
    }

    if (strcmp(argv[1], "execute") == 0)
    {
        if (strcmp(argv[1], "execute") == 0)
        {
            if (strcmp(argv[3], "-u") == 0)
            {
                char pid_command[256];
                snprintf(pid_command, sizeof(pid_command), "%d", getpid());

                char buffer[556]; // 256 for pid_command, 300 for command
                strcpy(buffer, pid_command);
                strcat(buffer, " ");
                strcat(buffer, argv[2]); // Copy pid_command to buffer

                char command[300] = ""; // Initialize command to an empty string

                for (int i = 4; i < argc; i++)
                {
                    strcat(command, " ");     // Add a space before each argument
                    strcat(command, argv[i]); // Add the argument to the command
                }

                strcat(buffer, command);

                int length = strlen(buffer);
                write(write_pipe, &length, sizeof(length));
                write(write_pipe, buffer, length);
                printf("1 Task Received\n");
            }
        }
        if (strcmp(argv[3], "-p") == 0)
        {
            char pid_command[256];
            snprintf(pid_command, sizeof(pid_command), "%d", getpid());

            char buffer[556];
            strcpy(buffer, pid_command);
            strcat(buffer, " ");
            strcat(buffer, argv[2]);

            char command[300] = "";

            for (int i = 4; i < argc; i++)
            {
                strcat(command, " ");
                strcat(command, argv[i]);
            }

            strcat(buffer, command);

            int length = strlen(buffer);
            write(write_pipe, &length, sizeof(length));
            write(write_pipe, buffer, length);

            char *command_copy = strdup(command);
            char *token = strtok(command_copy, "|");
            int command_count = 0;
            while (token != NULL)
            {
                command_count++;
                token = strtok(NULL, "|");
            }
            printf("%d Tasks Received\n", command_count); // Print the number of commands sent

            free(command_copy);
        }
    }

    if (strcmp(argv[1], "status") == 0)
    {

        // If the user requested the status, write it to the pipe
        char pipe_name[128];
        int pid = getpid();
        char pid2[258];
        snprintf(pipe_name, sizeof(pipe_name), "/tmp/my_pipe_%d", pid);
        snprintf(pid2, sizeof(pid2), "%d ", pid);

        char buffer[300]; // Buffer to hold the combined message
        strcpy(buffer, pid2);
        strcat(buffer, " ");
        strcat(buffer, "0");
        strcat(buffer, " ");     // Copy pid2 to buffer
        strcat(buffer, argv[1]); // Append argv[1] to buffer

        int length = strlen(buffer);                // Get the length of the message
        write(write_pipe, &length, sizeof(length)); // Write the length to the pipe
        write(write_pipe, buffer, length);          // Write the message to the pipe

        printf("Pipe_name :%s\n", pipe_name);
        sleep(5);
        int pipe_read = -1;
        while (pipe_read == -1)
        {
            pipe_read = open(pipe_name, O_RDONLY);
            if (pipe_read == -1)
            {
                perror("Error opening pipe");
                sleep(1);
            }
        }

        char buffer2[256];
        while (1)
        {
            ssize_t count = read(pipe_read, buffer2, sizeof(buffer2) - 1);
            if (count > 0)
            {
                buffer2[count] = '\0';
                printf("%s", buffer2);
                break;
            }
            else if (count == 0)
            {
                printf("No Status, Womp Womp");
                break;
            }
            else
            {
                perror("read");
                return 1;
            }
        }
        close(pipe_read);
        close(write_pipe);
        close(read_pipe);

        return 0;
    }
}
