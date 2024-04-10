// processOne.c
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <signal.h>
#include "message.h"

void signalWait()
{
    sigset_t set;
    int sig_num;

    // return;
    sigemptyset(&set);
    sigaddset(&set, SIGUSR1);
    sigprocmask(SIG_BLOCK, &set, NULL);
    sigwait(&set, &sig_num);
}

// void signal_handler(int signum) {
//     // Обработка сигнала
//     printf("Процесс P1: сигнал от P2 получен\nПроцесс P1: обработка данных из канала\nПроцесс P1: завершение работы\n");

//     // Завершение работы после обработки сигнала
//     exit(EXIT_SUCCESS);
// }

int main(int argc, char **argv)
{
    Message msg;
    int pipe_fd;

    if (argc != 3)
    {
        fprintf(stderr, "Usage: %s <pipe_name> <message>\n", argv[0]);
        exit(EXIT_FAILURE);
    }

    // Prepare the message
    msg.process_id = 0;
    strncpy(msg.text, argv[2], sizeof(msg.text));

    // Wait for signal from the second process
    printf("P%d: Waiting for signal from the second process...\n", msg.process_id);
    signalWait();
    //  signal(SIGUSR1, signal_handler);
    printf("P%d: Signal received.\n", msg.process_id);

    // Open the named pipe for writing
    pipe_fd = open(argv[1], O_RDWR);
    if (pipe_fd == -1)
    {
        perror("open");
        exit(EXIT_FAILURE);
    }

    // Write message to the pipe
    if (write(pipe_fd, &msg, sizeof(Message)) == -1)
    {
        perror("write");
        exit(EXIT_FAILURE);
    }

    // Close the pipe
    close(pipe_fd);

    // Continue with the rest of the process
    // processBehavior(msg, fd);

    printf("P%d: Shutdown.\n", msg.process_id);
    exit(EXIT_SUCCESS);
}
