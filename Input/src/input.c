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
 * This function runs a command prompt for user to step through the state machine.
 * Upon error, 's', or 'es' command, this process will terminate.
 */
int main(void) {
	// Connect to named end point
	int server_coid = name_open("controller", 0);
	if(server_coid == -1) {
		printf("Error: Cannot name_open");
		return EXIT_FAILURE;
	}

	State currentState = READY;
	// Initialize controllers
	if(MsgSend(server_coid, &currentState, sizeof(currentState), NULL, 0) == -1){
		printf("Error: MsgSend failed.\n");
		name_close(server_coid);
		exit(EXIT_FAILURE);
	}

	// Run machine
	char command[3];
	while(1){
		// Prompt for next event
		printf("\nEnter the input (ld, lu, rd, ru, p, s, es):\n");
		scanf("%s", &command, &pause);

		// assign new state
		int found = 0;
		for(int i=0; i<NUM_STATES; i++){
			if(strcmp(command, inMessage[i])==0){
				found++;
				currentState = i;
				break;
			}
		}

		// Invalid command
		if(found == 0){
			printf ("\nInvalid command\n");
			continue;
		}

		// Pause input do not send
		if(strcmp(command, inMessage[PAUSE]) == 0){
			int pause = 0;
			scanf("%d", &pause);
			printf("Pause for %d seconds...\n", pause);
			sleep(pause);
		}else{
			// Send message and wait for reply
			if(MsgSend(server_coid, &currentState, sizeof(currentState), NULL, 0) == -1){
				printf("\nServer responded with ERROR\n");
				name_close(server_coid);
				exit(EXIT_FAILURE);
			}
			// Check if command was exit
			if(currentState == EXIT || currentState == EMERGENCY) break;
		}
	}

	printf("\nExiting Input...\n");
	name_close(server_coid);
	return EXIT_SUCCESS;
}
