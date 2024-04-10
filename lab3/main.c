#include <stdio.h>
#include <string.h>

#include "messageExchanger.h"


int main(int argc, char** argv) {
	char P0_msg[MSG_SIZE + 1], P1_msg[MSG_SIZE + 1];

	strcpy(P0_msg, "Default msg from process 0.");
	strcpy(P1_msg, "Default msg from process 1.");

	switch (argc) {
		case 3:
			if (strlen(argv[2]) <= MSG_SIZE)
				strcpy(P1_msg, argv[2]);
			else {
				fprintf(stderr, "Error: data size can't be more than %hhu characters!\n", MSG_SIZE);
				break;
			}
		case 2:
			if (strlen(argv[1]) <= MSG_SIZE)
				strcpy(P0_msg, argv[1]);
			else {
				fprintf(stderr, "Error: data size can't be more than %hhu characters!\n", MSG_SIZE);
				break;
			}
		default: startCommunication(P0_msg, P1_msg); break;
	}

    return 0;
}
