#ifndef MESSAGEEXCHANGER_H
#define MESSAGEEXCHANGER_H

#define MSG_SIZE 40		// Размер сообщения, передаваемого процессами-потомками

void startCommunication(char P0_msg[MSG_SIZE + 1], char P1_msg[MSG_SIZE + 1]);

#endif
