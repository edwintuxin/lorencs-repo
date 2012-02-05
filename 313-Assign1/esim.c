/*
 * main.c
 *
 *  Created on: Feb 4, 2012
 *      Author: Mik
 */
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <math.h>
#include "esim.h"

/*
 *
 * TODO: - get random generator working
 * 		 - finish bitError function
 * 		 - finish transmit function
 *
 *
 */
/* input global variables */
int A;						// feedback time
int K;						// number of blocks
int F;						// size of frame
double e;					// probability of bit error
int R;						// length of simulation
int R2;						// copy of R to return at the end of simulation
int T;						// number of trials
int t1, t2, t3, t4, t5;		// seeds for each trial

/* stats variables */
int transmitCount;			// number of total transmissions
int successCount;		// number of successful transmissions

int main(int argc, char* argv[]){
	checkInput(argc, argv);

	/* scan in input variables */
	sscanf(argv[1], "%d", &A);
	sscanf(argv[2], "%d", &K);
	sscanf(argv[3], "%d", &F);
	sscanf(argv[4], "%g", &e);
	sscanf(argv[5], "%d", &R);
	sscanf(argv[6], "%d", &T);
	sscanf(argv[7], "%d", &t1);
	sscanf(argv[8], "%d", &t2);
	sscanf(argv[9], "%d", &t3);
	sscanf(argv[10], "%d", &t4);
	sscanf(argv[11], "%d", &t5);
	R2 = R;
	transmitCount = 0;
	successCount = 0;

	runSim();

	return 0;
}

void runSim(){
	int blockSize;
	int r = 0;

	if (K > 0){
		blockSize = F/K;
		r = getr(blockSize);
		blockSize = blockSize + r;
	} else {
		blockSize = F;
		K = 1;
	}

	// until bit time runs out
	while (R > 0){
		int result = 0;

		//until a successful transmission
		while (result == 0){
			transmitCount++;
			result = transmit(blockSize);
		}
		successCount++;

		//R = 0;
	}
}

int getr(int blockSize){
	int r = 0;
	while (blockSize + r + 1 > pow(2,r)){
		r++;
	}
	return r;
}

int transmit(int blockSize){
	int result = 1;
	int bitErrors;

	// determine the amount of bit errors in each block
	// and "send" each block (subtract blockSize from R)
	for (int i = 0; i < K; i++){
		bitErrors = 0;

		// generate an error for each bit with probability e
		for (int j = 0; j < blockSize; j++){
			if (bitError()){
				bitErrors++;
			}
		}

		R = R - blockSize;
		printf("%d\n", R);
	}

	return result;
}

// returns 1 if bit error, 0 otherwise
int bitError(){
	double random;
	if (random > e){
		return 1;
	} else {
		return 0;
	}
}

void checkInput(int argc, char* argv[]){
	if (argc < 12){
		printf("Warning: Not enough arguments to 'esim', please re-run with the right amount of arguments.\n");
		exit(0);
	}

	sscanf(argv[2], "%d", &K);
	sscanf(argv[3], "%d", &F);

	if (K == 0){
		return;
	}

	if (F%K != 0){
		printf("Warning: F is not divisible by K, exiting simulation.\n");
		exit(0);
	}
}

