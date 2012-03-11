/*
 * psim.h
 *
 *  Created on: Mar 10, 2012
 *      Author: lorencs
 */

#include "memwatch.h"

#ifndef PSIM_H_
#define PSIM_H_

#endif /* PSIM_H_ */

// struct to hold info about the N station
typedef struct{
	int frameQ;			// count that indicates how many frames are waiting to be transmitted
} station;

//functions
void checkInput(int argc, char* argv[]);
void runSim();
void generateFrames();
