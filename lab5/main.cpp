#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>  // for errno
#include <string.h> // for strncpy()
#include <sys/wait.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/sem.h>
#include <sys/msg.h>

#pragma region Global vars
#define FORK_ERROR 1 // Exit status for failed fork()
#define BUFSIZE 64   // I think that's enough for my input
#define MSGTYPE_ANY 0
#define IPC_NOFLAGS 0
#pragma endregion

typedef unsigned char byte; // this is missing in standard libs for whatever reason.
typedef struct
{
    byte flags;
    byte code;
} wait_t; // union for wait result
typedef struct
{
    char *command;   // this is for compability with param_start_t
    bool odd;        // is process odd or not
    int semid, mqid; // semaphore and message queue IDs
} param_t;           // Struct for child process execution data.
typedef struct
{
    long type;                       // this is for combatibility with POSIX syscalls
    char text[BUFSIZE];              // useful data
} msg;                               // Struct for data to be sent via message queue.
const char *filename = "poetry.txt"; // file with text
msg mbuf;                            // temporary store of message queue record
// Semaphore-specific actions for System V: A(S, N), D(S, N), Z(S)
#define sem_a(sem_id, sb, i, n) \
    sb.sem_num = (i);           \
    sb.sem_op = (n);            \
    semop(sem_id, &sb, 1);
#define sem_d(sem_id, sb, i, n) \
    sb.sem_num = (i);           \
    sb.sem_op = -(n);           \
    semop(sem_id, &sb, 1);
#define sem_z(sem_id, sb, i) \
    sb.sem_num = (i);        \
    sb.sem_op = 0;           \
    semop(sem_id, &sb, 1);
// Fill param_t with child process execution data.
void create_param(param_t *param, int sem_id, int mq_id, bool odd = true)
{
    param->command = "process_poetry"; // this is safe to set -Wno-write-strings
    param->semid = sem_id;
    param->mqid = mq_id;
    param->odd = odd;
}
// Create new thread in new process via fork() and run callback in it.
pid_t fork_thread(int(func(void *param)), void *param)
{
    int childpid;
    switch (childpid = fork())
    {
    case -1:
        fprintf(stderr, "[shell] Could not fork to subcommand '%s'\n", *(char **)param);
        exit(FORK_ERROR);
        break;
    case 0:
        exit(func(param)); /* run callback and exit */
        break;
    default:
        fprintf(stderr, "[shell] Process %d created for '%s'\n", childpid, *(char **)param);
        return childpid;
        break;
    }
}
// Wait till ALL running subcommands end
int join_threads()
{
    pid_t wait_pid;
    wait_t wait_ret;
    int exec_result = 0;
    while ((wait_pid = wait(NULL)) > 0)
    {
        printf("[shell] Process %d exited with code %d\n", wait_pid, wait_ret.code);
        exec_result |= wait_ret.code; // OR all codes
    }
    return exec_result;
}
// Callback function for thread to process input text file.
int process_poetry(void *_param)
{
    param_t *param = (param_t *)_param; // C style type conversion
    FILE *f = fopen(filename, "r");     // try to open poetry file
    if (f == NULL)
        return EXIT_FAILURE; // immediately fail on non-existent file
    char buf[BUFSIZE];
    int num = 0;              // current line number
    struct sembuf sb;         // semaphore control
    sb.sem_flg = IPC_NOFLAGS; // flags are usually unset
    mbuf.type = 1;            // this MUST be positive, as said in man

    char *ptr = NULL;
    do
    {
        do
        {
            num++;                            // increase current line number
            ptr = fgets(buf, BUFSIZE - 1, f); // read line from file
            if (ptr == NULL)
                break; // stop on eof
            if ((num % 2) != param->odd)
                continue; // skip not owned lines
            /* используем два семафора. стартовое состояние: 0 1
               первый процесс нечётный, второй чётный. 0 - доступно, 1 - заблокировано */
            sem_z(param->semid, sb, (short)param->odd); // wait till current is unblocked
            // send message

            strncpy(mbuf.text, buf, BUFSIZE - 1);
            mbuf.text[strlen(buf) - 1] = '\0';                          // remove line ending
            if (msgsnd(param->mqid, &mbuf, BUFSIZE, IPC_NOFLAGS) == -1) // try to send
                return errno;                                           // immediately fail on mq-errors
            sem_a(param->semid, sb, (short)param->odd, 1);              // block itself
            sem_d(param->semid, sb, (short)!param->odd, 1);             // unblock partner
        } while ((num % 4) != 0);                                       // break every 4 lines
        fgets(buf, BUFSIZE - 1, f);                                     // read separator line
    } while (ptr != NULL);                                              // stop on eof
    return EXIT_SUCCESS;
}

// Main entry point of application.
int main(int argc, char **argv)
{
    key_t mk = IPC_PRIVATE; // use private key to simplify task
    int mq = msgget(mk, IPC_CREAT | 0600);
    if (mq == -1)
    {
        printf("Could not get message queue\n");
        return EXIT_FAILURE;
    }
    int sid = semget(mk, 2, IPC_CREAT | 0600); // get two semaphores
    if (sid == -1)
    {
        printf("Could not get semaphore set\n");
        return EXIT_FAILURE;
    }
    struct sembuf sb;
    sb.sem_flg = IPC_NOFLAGS;                            // don't use flags
    sem_a(sid, sb, 0, 1);                                // increase semaphore 0 value to 1 (init state: 0 1)
    param_t *param = (param_t *)malloc(sizeof(param_t)); // C style
    create_param(param, sid, mq, true);                  // init odd thread
    fork_thread(process_poetry, (void *)param);          // run odd thread
    create_param(param, sid, mq, false);                 // init non-odd thread
    fork_thread(process_poetry, (void *)param);          // run non-odd thread
    join_threads();                                      // wait for children to do all the hard work

    ssize_t cnt;
    do
    {
        cnt = msgrcv(mq, &mbuf, BUFSIZE, MSGTYPE_ANY, IPC_NOWAIT);
    } while (cnt != -1 && printf("%s\n", mbuf.text));

    return EXIT_SUCCESS;
}