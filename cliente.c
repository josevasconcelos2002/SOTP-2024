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

    char pid_command[256];
    snprintf(pid_command, sizeof(pid_command), "%d ", getpid());

    if(strcmp(argv[1],"execute") == 0){
        if(strcmp(argv[3],"-u") == 0){
            for (int i = 4; i < argc; i++){
                write(write_pipe, argv[i], strlen(argv[i]));
                if(i < argc - 1){
                    write(write_pipe, " ", 1);
                }
            }
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
        write(write_pipe, argv[1], strlen(argv[1]));
        char buffer[256];
        while (1) {
            ssize_t count = read(read_pipe, buffer, sizeof(buffer)-1);
            if (count > 0) {
                buffer[count] = '\0';
                printf("%s", buffer);
                break;
            } else if (count == 0) {
                break;
            } else {
                perror("read");
                return 1;
            }
        }
    }

    close(write_pipe);
    close(read_pipe);

    return 0;
}