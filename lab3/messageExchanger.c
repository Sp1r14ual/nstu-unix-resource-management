#include "messageExchanger.h"

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>
#include <string.h>

#include <unistd.h>
#include <sys/wait.h>
#include <sys/types.h>
#include <signal.h>

#define PROCESS_COUNT   2		// Кол-во процессов

#define INCOMPLETE_DATA 0		// Неполные данные при пересылке
#define NO_OPTIONS		0		// Ожидание процесса без указанных опций

#define ERR  -1					// Код ошибки
#define CHILD 0					// Id процесса-потомка

#define RD	  0					// Чтение из канала
#define WR	  1					// Запись в канал
#define RD_WR 2					// Количество операций в канале

#define MCS_WAIT_TIME 100000	// Время ожидания начала работы второго процесса


typedef struct Message {		// Структура обрабатываемых данных
	uint32_t process_id;		// Id процесса-отправителя
	char text[MSG_SIZE + 1];	// Текст сообщения
} Message;

// Формирование сообщений для отправки процессами-потомками
Message createMessage(uint32_t process_id,  char text[MSG_SIZE + 1]) {
	Message new_message;

	new_message.process_id = process_id;
	strcpy(new_message.text, text);

	return new_message;
}

// Ожидание работы процесса, пока не получен сигнал
static void signalWait() {
	int sig_num;
	sigset_t set;

	sigemptyset(&set);				// Инициализируем список сигналов
	sigaddset(&set, SIGUSR1);		// Добавляем сигнал в список
	sigprocmask(SIG_BLOCK, &set, NULL);		// Блокируем сигнал
	sigwait(&set, &sig_num);		// Останавиливаем выполнение процесса
}									// пока не будет принят сигнал из набора

// Основное поведение процесса -- передача данных в канал сообщений
static void processBehavior(Message msg, int fd[RD_WR]) {
	printf("P%d:\tData preparation for sending to pipe.\n", msg.process_id);

	close(fd[RD]);							// Запись сообщения в канал
	write(fd[WR], &msg, sizeof(Message));
}

static void processOne(Message msg, int fd[RD_WR]) {
	printf("P%d:\tWait for signal.\n", msg.process_id);
	signalWait();

	printf("P%d:\tSignal received.\n", msg.process_id);
	processBehavior(msg, fd);

	printf("P%d:\tShutdown.\n", msg.process_id);
	exit(EXIT_SUCCESS);
}

static void processTwo(Message msg, int fd[RD_WR], pid_t receiver_pid) {
	usleep(MCS_WAIT_TIME);			// Увеличиваем время начала работы отправителя,
	processBehavior(msg, fd);		// чтобы предотвратить завершени его работы
									// до создания принимающего процесса
	printf("P%d:\tSending a signal to the next process.\n", msg.process_id);
	kill(receiver_pid, SIGUSR1);

	printf("P%d:\tShutdown.\n", msg.process_id);
	exit(EXIT_SUCCESS);
}

void startCommunication(char P0_msg[MSG_SIZE + 1], char P1_msg[MSG_SIZE + 1]) {
	pid_t pid[PROCESS_COUNT];		// Кол-во процессов потомков

	int fd[RD_WR];					// Канал обмена сообщениями
	pipe(fd);

	printf("Parent:\tPipe was created.\n");

	for (uint8_t i = 0; i != PROCESS_COUNT; ++i)	// Порождение процессов потомков
		if ((pid[i] = fork()) == ERR)
			fprintf(stderr, "Error: child fork error!\n");
		else
			if (pid[i] == CHILD) {					// Запуск потоков по их задачам
				printf("Parent:\tProcess %d was created.\n", i);

				switch (i) {
					case 0: processOne(createMessage(i, P0_msg), fd); break;
					case 1: processTwo(createMessage(i, P1_msg), fd, pid[0]);  break;
					default: break;
				}
			}

	uint8_t closed_processes = 0;				// Кол-во завершённых процессов-потомков
	while (closed_processes != PROCESS_COUNT) {	// Проверяем каналы на наличие сообщений
		close(fd[WR]);							// до тех пор, пока не завершены все процессы-потомки

		Message msg;
		ssize_t received = read(fd[RD], &msg, sizeof(Message));

		if (received > INCOMPLETE_DATA) {		// Если принято сообщение от процесса --
			++closed_processes;					// сохраняем результат в соответствующую переменную
												// и выводим сообщение на экран
			printf("Parent:\tProcessing data from process %d: %s\n", msg.process_id, msg.text);
		}
	}

	printf("Parent:\tWaiting for process to complete.\n");

	bool correct = true;
	for (uint8_t i = 0; i != PROCESS_COUNT; ++i)	// Приостанавливаем выполнение всех процессов-потомков
		if (pid[i] != waitpid(pid[i], NULL, NO_OPTIONS)) {
			correct = false;
			fprintf(stderr, "Error: child %hhu isn't done!\n", i);
		}

	if (correct)
		printf("Parent:\tAll processes are done.\n");
}
