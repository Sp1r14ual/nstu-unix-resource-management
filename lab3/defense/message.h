// message.h
#ifndef MESSAGE_H
#define MESSAGE_H

#define MSG_SIZE 40

typedef struct Message {
    int process_id;
    char text[MSG_SIZE + 1];
} Message;

#endif /* MESSAGE_H */
