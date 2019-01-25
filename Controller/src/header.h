/*
 * header.h
 *
 *  Created on: Nov 21, 2018
 *      Author: Nathan Festoso
 */

#ifndef ASS2_HEADER_H_
#define ASS2_HEADER_H_

#define NUM_STATES 12
typedef enum {
	READY = 		0,
	LEFT_DOWN =		1,
	LEFT_UP = 		2,
	RIGHT_DOWN =	3,
	RIGHT_UP =		4,
	ARMED = 		5,
	PUNCH = 		6,
	EXIT = 			7,
	START = 		8,
	STOP = 			9,
	PAUSE = 		10,
	EMERGENCY = 	11
} State;

const char *inMessage[NUM_STATES] = {
	"",
	"ld",
	"lu",
	"rd",
	"ru",
	"",
	"",
	"s",
	"",
	"",
	"p",
	"es"
};

const char *outMessage[NUM_STATES] = {
	"Ready...",
	"Left button down - press right button to arm press",
	"",
	"Right button down - press left button to arm press",
	"",
	"DANGER - Press is Armed! - Hold buttons for 2 seconds...",
	"Press Cutting Now",
	"Powering down...",
	"",
	"",
	"",
	"Emergency Stop!"
};

struct Machine{
	State currentState;
} typedef Machine_t;


#endif /* ASS2_HEADER_H_ */
