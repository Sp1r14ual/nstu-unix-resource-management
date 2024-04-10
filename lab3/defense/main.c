// main.c
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>
#include <fcntl.h>
#include "message.h"

#define PROCESS_COUNT   2
#define PIPE_NAME "communication_pipe"

int main(int argc, char** argv) {
    pid_t pid[PROCESS_COUNT];
    int status;

    // Create named pipe for communication
    if (mkfifo(PIPE_NAME, 0666) == -1) {
        perror("mkfifo communication_pipe");
        exit(EXIT_FAILURE);
    }

    for (int i = 0; i < PROCESS_COUNT; ++i) {
        if ((pid[i] = fork()) == -1) {
            perror("fork");
            exit(EXIT_FAILURE);
        } else if (pid[i] == 0) {
            // Child process
            switch (i) {
                case 0:
                    printf("Parent:\tProcess %d was created.\n", i);
                    execl("./processOne", "./processOne", PIPE_NAME, "Message from processOne", NULL);
                    perror("execl processOne");
                    exit(EXIT_FAILURE);
                case 1:
                    char str[20]; 
                    sprintf(str, "%d", pid[0]);
                    printf("Parent:\tProcess %d was created.\n", i);
                    execl("./processTwo", "./processTwo", PIPE_NAME, "Message from processTwo", str, NULL);
                    perror("execl processTwo");
                    exit(EXIT_FAILURE);
            }
        }
    }

    // Wait for all child processes to finish
    for (int i = 0; i < PROCESS_COUNT; ++i) {
        printf("Parent:\tWaiting for child process %d to finish.\n", i);
        if (waitpid(pid[i], &status, 0) == -1) {
            perror("waitpid");
            exit(EXIT_FAILURE);
        }
        printf("Parent:\tChild process %d has finished.\n", i);

        if (WIFEXITED(status)) {
            printf("Child %d exited with status %d\n", i, WEXITSTATUS(status));
        } else {
            printf("Child %d exited abnormally\n", i);
        }
    }

    // Remove named pipe
    if (unlink(PIPE_NAME) == -1) {
        perror("unlink communication_pipe");
        exit(EXIT_FAILURE);
    }

    return EXIT_SUCCESS;
}
