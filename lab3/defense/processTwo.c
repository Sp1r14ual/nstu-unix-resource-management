// processTwo.c
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <signal.h>
#include "message.h"

int main(int argc, char **argv)
{
    Message msg;
    int pipe_fd;
    pid_t receiver_pid;

    if (argc != 4)
    {
        fprintf(stderr, "Usage: %s <pipe_name> <receiver_pid> <message>\n", argv[0]);
        exit(EXIT_FAILURE);
    }

    // Extract receiver PID from command line arguments
    receiver_pid = (pid_t)atoi(argv[3]);

    // Open the named pipe for writing
    pipe_fd = open(argv[1], O_RDWR);
    if (pipe_fd == -1)
    {
        perror("open");
        exit(EXIT_FAILURE);
    }

    // Prepare the message
    msg.process_id = 1;
    strncpy(msg.text, argv[2], sizeof(msg.text));

    // Write message to the pipe
    if (write(pipe_fd, &msg, sizeof(Message)) == -1)
    {
        perror("write");
        exit(EXIT_FAILURE);
    }

    // Close the pipe
    close(pipe_fd);

    // Send signal to the first process
    printf("P%d: Sending signal to the first process.\n", msg.process_id);
    kill(receiver_pid, SIGUSR1);

    printf("P%d: Shutdown.\n", msg.process_id);
    exit(EXIT_SUCCESS);
}
