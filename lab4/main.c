#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <errno.h>

void redirectOutput(int fd) {
    dup2(fd, STDOUT_FILENO); // Перенаправление стандартного вывода
    dup2(fd, STDERR_FILENO); // Перенаправление вывода ошибок
    close(fd);
}

int main() {
    pid_t pid1, pid2;
    int status1, status2;

    printf("Создание процесса для первой команды\n");
    if ((pid1 = fork()) < 0) {
        perror("Ошибка при создании процесса для первой команды");
        exit(EXIT_FAILURE);
    } else if (pid1 == 0) { // дочерний процесс для первой команды
        printf("Выполнение первой команды: cc pr1.c\n");
        if (execlp("cc", "cc", "pr1.c", NULL) == -1) {
            perror("Ошибка при выполнении первой команды");
            // В случае неудачи выполнения cc, выполняем cat с перенаправлением вывода в файл
            int fd = open("pr1.txt", O_WRONLY | O_CREAT | O_TRUNC, 0666);
            if (fd == -1) {
                perror("Ошибка при открытии файла pr1.txt");
                exit(EXIT_FAILURE);
            }
            redirectOutput(fd); // Перенаправление вывода на файл и на экран
            execlp("cat", "cat", "pr1.c", NULL);
            perror("Ошибка при выполнении команды cat");
            exit(EXIT_FAILURE);
        }
    } else { // родительский процесс
        printf("Создание процесса для второй команды\n");
        if ((pid2 = fork()) < 0) {
            perror("Ошибка при создании процесса для второй команды");
            exit(EXIT_FAILURE);
        } else if (pid2 == 0) { // дочерний процесс для второй команды
            printf("Выполнение второй команды: cc pr2.c\n");
            if (execlp("cc", "cc", "pr2.c", NULL) == -1) {
                perror("Ошибка при выполнении второй команды");
                // В случае неудачи выполнения cc, выполняем cat с перенаправлением вывода в файл
                int fd = open("pr2.txt", O_WRONLY | O_CREAT | O_TRUNC, 0666);
                if (fd == -1) {
                    perror("Ошибка при открытии файла pr2.txt");
                    exit(EXIT_FAILURE);
                }
                redirectOutput(fd); // Перенаправление вывода на файл и на экран
                execlp("cat", "cat", "pr2.c", NULL);
                perror("Ошибка при выполнении команды cat");
                exit(EXIT_FAILURE);
            }
        } else { // родительский процесс
            // Ожидание завершения выполнения обоих процессов
            waitpid(pid1, &status1, 0);
            waitpid(pid2, &status2, 0);
            
            printf("Выполнение команд завершено\n");
        }
    }

    return 0;
}

