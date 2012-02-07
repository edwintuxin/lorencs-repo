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

//#define DEBUG

/* input global variables */
int A;						// feedback time
int K;						// number of blocks
int F;						// size of frame
float e;					// probability of bit error
int R;						// length of simulation
int R2;						// copy of R to restore after each trial
int T;						// number of trials
int t[5];					// seeds for each trial

/* stats variables */
int transmitCount;			// number of total transmissions
int successCount;			// number of successful transmissions
int runtime;				// time that each trial runs for (< R)
double averageFrameTx[5];	// average number of frames tx'd in each trial
double throughput[5];		// throughput of each trial
int K0;						// boolean that if true, means it is the special condition K = 0

int main(int argc, char* argv[]){
	checkInput(argc, argv);

	/* scan in input variables */
	sscanf(argv[1], "%d", &A);
	sscanf(argv[2], "%d", &K);
	sscanf(argv[3], "%d", &F);
	sscanf(argv[4], "%f", &e);
	sscanf(argv[5], "%d", &R);
	sscanf(argv[6], "%d", &T);
	sscanf(argv[7], "%d", &t[0]);
	sscanf(argv[8], "%d", &t[1]);
	sscanf(argv[9], "%d", &t[2]);
	sscanf(argv[10], "%d", &t[3]);
	sscanf(argv[11], "%d", &t[4]);
	R2 = R;

	printf("\n");

	// run simulation T times, reset variables for each trial
	for (int i = 0; i < T; i++){
		transmitCount = 0;
		successCount = 0;
		runtime = 0;
		srand(t[i]);
		R = R2;
		K0 = 0;
		runSim();
		averageFrameTx[i] = (double)transmitCount/(double)successCount;
		throughput[i] = ((double)F * (double)successCount)/(double)runtime;

		printf("###### Trial #%d\n#\n", i+1);
		printf("# Total transmissions:      %d\n", transmitCount);
		printf("# Successful transmissions: %d\n#\n", successCount);
		printf("# Total runtime: %d\n#\n", runtime);
		printf("################");
	}

	printStats(argc, argv);

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
		// set K to 1 to treat it like the K = 1 case, except no bit errors will be allowed
		K = 1;
		K0 = 1;
	}

	// until bit time runs out
	while (R > 0){
		int result = 0;

		//until a successful transmission
		while (result == 0){
			// stop running if not enough time to transmit the frame
			if (R < (K*blockSize + A)){
				goto breakLoop;
			}
			transmitCount++;
			result = transmit(blockSize);
		}
		successCount++;
	}
	breakLoop:;
}

int getr(int blockSize){
	int r = 0;
	while (blockSize + r + 1 > pow(2,r)){
		r++;
	}
	return r;
}

// transmit all the blocks, generate errors for them
// if there is more than 1 error in any of the blocks, result is 0
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

		//printf("%d bit errors in block %d of frame %d!\n", bitErrors, i + 1, successCount + 1);
		int checkval = 1;

		// if special case K = 0, 0 errors are allowed, else 1
		if (K0){
			checkval = 0;
		}

		if (bitErrors > checkval){
			result = 0;
		}

		// transmitting block
		runtime = runtime + blockSize;
		R = R - blockSize;
		//printf("send block: %d\n", runtime);
	}

	// transmit ACK
	runtime = runtime + A;
	//printf("send ack:   %d\n", runtime);
	R = R - A;

	return result;
}

// returns 1 if bit error, 0 otherwise
int bitError(int argc, char* argv[]){
	double random = (double)rand() / (double)RAND_MAX;
	if (random < e){
		return 1;
	} else {
		return 0;
	}
}

void printStats(int argc, char* argv[]){
	printf("Input parameters:\n");
	for (int i = 0; i < argc; i++){
		printf("%s ", argv[i]);
	}
	printf("\n\n");

	// calculate mean of average # of frames
	double mean = 0;
	for (int i = 0; i < T; i++){
		mean = mean + averageFrameTx[i];
	}
	mean = mean/T;

	printf("Average # of Frame Transmissions: %.3f with a 95%c confidence interval of ", mean, '%');
	printCI(mean, averageFrameTx);

	// calculate mean throughput
	mean = 0;
	for (int i = 0; i < T; i++){
		mean = mean + throughput[i];
	}
	mean = mean/T;

	printf("Throughput: %.3f with a 95%c confidence interval of ", mean, '%');
	printCI(mean, throughput);
}

void printCI(double mean, double* array){
	double expectedSum = 0;
	for (int i = 0; i < T; i++){
		expectedSum = expectedSum + pow((array[i] - mean), 2);
	}

	double stdDev = sqrt(expectedSum/(T-1));

	double error = 2.776*(stdDev/pow(T,1/2));
	double c1 = mean - error;
	double c2 = mean + error;

	printf("(%.3f, %.3f)\n", c1, c2);
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

