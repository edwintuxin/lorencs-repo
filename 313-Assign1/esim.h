/*
 * esim.h
 *
 *  Created on: Feb 5, 2012
 *      Author: Mik
 */

#ifndef ESIM_H_
#define ESIM_H_



#endif /* ESIM_H_ */

/* function prototypes */
void runSim();
void checkInput(int argc, char* argv[]);
int getr(int blockSize);
int transmit(int blockSize);
int bitError();
void printStats(int argc, char* argv[]);
void printCI(double mean, double* array);
