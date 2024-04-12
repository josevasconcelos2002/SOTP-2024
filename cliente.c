#include <stdio.h>
#include <string.h>
#include <fcntl.h>
#include <unistd.h>

int main(int argc, char *argv[]) {
    if(argc < 2){
        printf("Not Enough Arguments\n");
        return 1;
    }

    

    int read_pipe = open("/tmp/my_pipe_out", O_RDONLY);
    if (read_pipe == -1) {
        perror("open");
        return 1;
    }

    int write_pipe = open("/tmp/my_pipe", O_WRONLY);
    if (write_pipe == -1) {
        perror("open");
        return 1;
    }

 
    if(strcmp(argv[1],"execute") == 0){
        if(strcmp(argv[3],"-u") == 0){
            char pid_command[256];
            snprintf(pid_command, sizeof(pid_command), "%d", getpid());


            write(write_pipe,pid_command,strlen(pid_command));
            char buffer[1024] = {0}; // Buffer to hold the complete command
            strcat(buffer, pid_command); // Add the pid_command to the buffer

            for (int i = 4; i < argc; i++) {
                strcat(buffer, " "); // Add a space before each argument
                strcat(buffer, argv[i]); // Add the argument to the buffer
            }

            write(write_pipe, buffer, strlen(buffer));
        }
        if(strcmp(argv[3],"-p") == 0){
        char *token = strtok(argv[4], "|");
        while(token != NULL){
            char command[256];
            snprintf(command, sizeof(command), "%s|", token);
            write(write_pipe, command, strlen(command));
            token = strtok(NULL,"|");
            }
        }
    }

    if (strcmp(argv[1], "status") == 0) {

        // If the user requested the status, write it to the pipe
        char pipe_name[128];
        int pid = getpid();
        char pid2[258];
        snprintf(pipe_name, sizeof(pipe_name), "/tmp/my_pipe_%d", pid);
        snprintf(pid2, sizeof(pid2),"%d ", pid);
        write(write_pipe, pid2, strlen(pid2));
        write(write_pipe, argv[1], strlen(argv[1]));
        printf("Pipe_name :%s\n", pipe_name);
        sleep(5);
        int pipe_read = open(pipe_name, O_RDONLY);
        if(pipe_read == -1){
            perror("Error opening pipe");
            return 1;
        }
        char buffer[256];
        while (1) {
            ssize_t count = read(pipe_read, buffer, sizeof(buffer)-1);
            if (count > 0) {
                buffer[count] = '\0';
                printf("%s", buffer);
                break;
            } else if (count == 0) {
                break;
       while(pipe_read == -1){
            sleep(1);
        }     } else {
                perror("read");
                return 1;
            }
        }
        close(pipe_read);
    }

    close(write_pipe);
    close(read_pipe);

    return 0;
}
