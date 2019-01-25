#include <stdlib.h>
#include <stdio.h>
#include <sys/iofunc.h>
#include <sys/dispatch.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>

#include <time.h>
#include <sys/netmgr.h>
#include <sys/neutrino.h>

#include "header.h"

#define MY_PULSE_CODE   _PULSE_CODE_MINAVAIL

typedef union {
        struct _pulse   pulse;
} my_message_t;

int emergencyStop = -1;
int server_coid;
int released = -1;

void * doTimer(void *c) {
   struct sigevent         event;
   struct itimerspec       itime;
   timer_t                 timer_id;
   int                     chid;
   int                     rcvid;
   my_message_t            msg;

   chid = ChannelCreate(0);

   event.sigev_notify = SIGEV_PULSE;
   event.sigev_coid = ConnectAttach(ND_LOCAL_NODE, 0,
                                    chid,
                                    _NTO_SIDE_CHANNEL, 0);
   event.sigev_priority = SchedGet( 0, 0, NULL );
   event.sigev_code = MY_PULSE_CODE;
   timer_create(CLOCK_REALTIME, &event, &timer_id);

   State state = (int) c;

   long seconds =  ((state == PUNCH) ?  2 : 1);

   printf("Waiting for %d seconds..\n", seconds);

   itime.it_value.tv_sec = seconds;
   /* 500 million nsecs = .5 secs */
   itime.it_value.tv_nsec = 500000000;
   itime.it_interval.tv_sec = 1;
   /* 500 million nsecs = .5 secs */
   itime.it_interval.tv_nsec = 500000000;
   timer_settime(timer_id, 0, &itime, NULL);

   for (;;) {
       rcvid = MsgReceive(chid, &msg, sizeof(msg), NULL);
       if (rcvid == 0) { /* we got a pulse */
            if (msg.pulse.code == MY_PULSE_CODE) {
                timer_delete(timer_id);
                break;
            }
       }
   }

   if (released > 0) {
	   printf("One of the buttons were released!\n");
	   return NULL;
   }

   if (emergencyStop > 0) {
	   return NULL;
   }

   if (MsgSend(server_coid, &state, sizeof (state) + 1, NULL, 0) == -1) {
	   fprintf(stderr, "Error during MsgSend\n");
       perror(NULL);
   }
   return NULL;
}

void startTimer(State nextState) {
	pthread_create (NULL, NULL, doTimer, (void *)nextState);
}

int main(void) {
  // Attach to device
  name_attach_t * controllerDevice = name_attach(NULL, "controller", 0);
  if (controllerDevice == NULL) {
    printf("Error: Cannot name_attach\n");
    return EXIT_FAILURE;
  }

  if ((server_coid = name_open("display", 0)) == -1) {
	  printf("Error: name open failed\n");
  	return EXIT_FAILURE;
  }

  State rmsg;
  State reply = START;
  State response;
  State statenum = START;	// Beginning state

  int rcvid;

  while (1)
  {
	  if ((rcvid = MsgReceive(controllerDevice -> chid, &rmsg, sizeof(rmsg), NULL)) < 0) {
		  if (emergencyStop < 0) printf("Error: Receiving pulse\n");
		  return EXIT_FAILURE;
	  }

	  switch (rmsg)
	  {
    		case LEFT_DOWN:
    			printf("Left down\n");
    			if (statenum == RIGHT_DOWN) statenum = ARMED;
    			else statenum = LEFT_DOWN;
    			released = -1; // button is pressed
    			break;
    		case RIGHT_DOWN:
    			printf("Right down\n");
    			if (statenum == LEFT_DOWN) statenum = ARMED;
    			else statenum = RIGHT_DOWN;
    			released = -1; // button is pressed
    			break;
    		case LEFT_UP:
    			printf("Left up\n");
    			statenum = LEFT_UP;
    			released = 1; // button is released
    			break;
    		case RIGHT_UP:
    			printf("Right up\n");
    			statenum = RIGHT_UP;
    			released = 1; // button is released
    			break;
    		case READY:
    			printf("Ready\n");
    			statenum = READY;
    			sleep(3);
    			break;
    		case EMERGENCY:
    		    printf("Emergency Stop\n");
    		    emergencyStop = 1;
    		    statenum = EXIT;
    		    break;
    		case EXIT:
    			printf("Exit\n");
    			statenum = rmsg;
    			break;
    		default:
    			statenum = -1;
   		}
      // assign a new state to the reply
      reply = statenum;
      // reply to the input micro-service
      MsgReply (rcvid, EOK, &reply, sizeof (reply));
      // send a state to the display
      if ((statenum > 0) && (statenum != LEFT_UP) && (statenum != RIGHT_UP)) {
      	// send a message to the display
      	rmsg = statenum;
      	if (MsgSend(server_coid, &rmsg, sizeof (rmsg) + 1, &response, sizeof (response)) == -1) {
      		fprintf(stderr, "Error during MsgSend\n");
      		perror(NULL);
      	}
      }
      // exit the process and send a kill packet to the display service
      if (statenum == EXIT) sleep(5);
      else if (statenum == ARMED) {
      	rmsg = PUNCH;
      	startTimer(rmsg);
      	statenum = READY; // go back to start
      }
      if (statenum == EXIT) break;
  }

  name_close(server_coid);
  name_detach(controllerDevice, NAME_FLAG_DETACH_SAVEDPP);
  printf("Controller exited.\n");
  return EXIT_SUCCESS;
}
