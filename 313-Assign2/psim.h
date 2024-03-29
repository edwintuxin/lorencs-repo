/*
 * psim.h
 *
 *  Created on: Mar 10, 2012
 *      Author: lorencs
 */

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <math.h>
#include "frame.h"
#include "memwatch.h"

#ifndef PSIM_H_
#define PSIM_H_

#endif /* PSIM_H_ */

// struct to hold info about the N station
typedef struct{
	int frameQ;					// count that indicates how many frames are waiting to be transmitted
	int frameTx;				// count of how many frames this station has transmitted
	int frameTotal;				// total number of generated frames
	frameList *pendingFrames;	// linked list of pending frames
	int *frameDelay;			// dynamic array of the delays of successfully transmitted frames
	int arraySize;				// keeps track of current size of the frameDelay array
	int tryingToTx;				// for protocol P: 0 if station is trying to tx for the first time, 1 if station is picking a slot to tx in
								// for protocol I: -1 if station is trying to tx for the first time, offset 0 to N if station has picked slot to tx in
	int intExp;					// for protocol B, an interval will be selected from 1 to 2^intExp

	double throughput[5];		// throughput of the station at each trial
	double avgDelay[5];			// avg delay of the station at each trial
	double outsRatio[5];		// ratio of outstanding frame at each trial
} station;

//functions
void checkInput(int argc, char* argv[]);
void runSim();
void generateFrames();
void initStations();
double getAvgDelay(int *array, int size);
int txNextSlot();
int txSlotOffset(int upperBound);
void transmitFrame (int stationId);
void copyNextToCurr();
void printStats(int argc, char* argv[]);
void printCI(double mean, double* array, int flag);
void cleanup();
