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
	int frameQ;					// count that indicates how many frames are waiting to be transmitted
	int frameTx;				// count of how many frames this station has transmitted
	frameList *pendingFrames;	// linked list of pending frames
	int *frameDelay;			// dynamic array of the delays of successfully transmitted frames
	int arraySize;				// keeps track of current size of the frameDelay array

	double throughput[5];		// throughput of the station at each trial
	double avgDelay[5];			// avg delay of the station at each trial
} station;

//functions
void checkInput(int argc, char* argv[]);
void runSim();
void generateFrames();
void initStations();
double getAvgDelay(int *array, int size);
void printStats(int argc, char* argv[]);
void printCI(double mean, double* array, int flag);
void cleanup();
