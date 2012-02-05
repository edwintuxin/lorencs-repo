/*
 * main.c
 *
 *  Created on: Feb 4, 2012
 *      Author: Mik
 */
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

/* global variable */
int A;						// feedback time
int K;						// number of blocks
int F;						// size of frame
double e;					// probability of bit error
int R;						// length of simulation
int T;						// number of trials
int t1, t2, t3, t4, t5;		// seeds for each trial

int main(int argc, char* argv[]){
	/* scan in input variables */
	sscanf(argv[1], "%d", &A);
	sscanf(argv[2], "%d", &K);
	sscanf(argv[3], "%d", &F);
	sscanf(argv[4], "%f", &e);
	sscanf(argv[5], "%d", &R);
	sscanf(argv[6], "%d", &T);
	sscanf(argv[7], "%d", &t1);
	sscanf(argv[8], "%d", &t2);
	sscanf(argv[9], "%d", &t3);
	sscanf(argv[10], "%d", &t4);
	sscanf(argv[11], "%d", &t5);

	return 0;
}

