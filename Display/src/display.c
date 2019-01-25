#include <stdlib.h>
#include <stdio.h>
#include <sys/iofunc.h>
#include <sys/dispatch.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <errno.h>
#include <sys/neutrino.h>
#include <sys/netmgr.h>

#include "header.h"

/*
 * Main method:
 *
 * This function handles output messages to user when it receives a state change from the controller.
 */
int main(void) {
	// Create channel
	name_attach_t *display;
	display = name_attach(NULL, "display", 0);
	if (display == NULL){
		perror("Error: name_attach failed.");
		exit (EXIT_FAILURE);
	}

	State currentState = READY;
	int rcvid;
	while(1){
		// Receive state change
		if((rcvid = MsgReceive(display->chid, &currentState, sizeof(currentState), NULL)) == -1){
			printf("Error: MsgReveice failed");
			name_detach(display, NAME_FLAG_DETACH_SAVEDPP);
			exit(EXIT_FAILURE);
		}

		// Print message
		printf("%s\n", outMessage[currentState]);

		// respond to controller
		MsgReply (rcvid, EOK, &currentState, sizeof(currentState));
		if(currentState == EXIT || currentState == EMERGENCY) break;
	}

	// Terminate
	printf("Exiting Display...\n");
	name_detach(display, NAME_FLAG_DETACH_SAVEDPP);
	return EXIT_SUCCESS;
}
