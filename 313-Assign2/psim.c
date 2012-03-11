/*
 * psim.c
 *
 *  Created on: Mar 10, 2012
 *      Author: lorencs
 */

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <math.h>
#include "psim.h"
#include "memwatch.h"

#define DEBUG

/* input global variables */
char Protocol;				// type of protocol
int N;						// number of stations
double p;					// probability of frame generation
int R;						// number of slots to simulate
int T;						// number of trials
int t[5];					// array of 5 seeds for 5 trials

/* other global vars */
station *Stations;
double throughput[5];		// array of throughputs for each trial
int frameTx;				// # of successful transmissions (in one trial)

int main(int argc, char* argv[]){
	checkInput(argc, argv);

	/* scan in input variables */
	sscanf(argv[1], "%c", &Protocol);
	sscanf(argv[2], "%d", &N);
	sscanf(argv[3], "%f", &p);
	sscanf(argv[4], "%d", &R);
	sscanf(argv[5], "%d", &T);
	sscanf(argv[6], "%d", &t[0]);
	sscanf(argv[7], "%d", &t[1]);
	sscanf(argv[8], "%d", &t[2]);
	sscanf(argv[9], "%d", &t[3]);
	sscanf(argv[10], "%d", &t[4]);

	Stations = malloc(N*sizeof(station));

	// run simulation T times
	for (int i = 0; i < T; i++){
		frameTx = 0;
		srand(t[i]);
		initStations();

		runSim();

		throughput[i] = frameTx/R;
		for (int j = 0; j < N; j++){
			Stations[j].throughput[i] = Stations[j].frameTx/R;
		}
	}

	printStats(argc, argv);

	return 0;
}


void runSim(){
	int slot = 0;		// count of how many slot times have elapsed

	while (slot < R){
		// go through entire station bus
		for (int i = 0; i < N; i++){
			//end simulation if R slots have elapsed
			if (slot > R){
				break;
			}

			generateFrames();

			// behave according to the specified protocol
			switch(Protocol){
				case 'T':
					// if station corresponding to slot has frame(s) to tx
					// transmit one frame
					if (Stations[i].frameQ > 0){
						frameTx++;
						Stations[i].frameTx++;
						Stations[i].frameQ--;
					}

					break;
				case 'P':

					break;

				case 'I':

					break;

				case 'B':

					break;
			}

			slot++;
		}
	}

}

// resets/initializes  all stations
void initStations(){
	for (int i = 0; i < N; i++){
		Stations[i].frameQ = 0;
	}
}

// generates frames for all N stations based on probability p
void generateFrames(){
	for (int i = 0; i < N; i++){
		double random = (double)rand() / (double)RAND_MAX;
		if (random < p){
			Stations[i].frameQ++;
		}
	}
}

// print out the end of execution statistics
void printStats(int argc, char* argv[]){
	for (int i = 1; i < 5; i++){
		printf("%s ", argv[i]);
	}
	printf("\n");

	// calculate mean of throughput
	double mean = 0;
	for (int i = 0; i < T; i++){
		mean = mean + throughput[i];
	}
	mean = mean/T;

	#ifdef DEBUG
		printf("avg throughput: ");
	#endif

	printf("%f ", mean);
	printCI(mean, throughput, 0);
	printf("\n");



	for (int i = 0; i < N; i++){

		#ifdef DEBUG
			printf("[%d] throughput: ", i);
		#endif
		// calculate mean of throughput
		mean = 0;
		for (int j = 0; j < T; j++){
			mean = mean + Stations[i].throughput[j];
		}
		mean = mean/T;

		printf("%f ", mean);
		printCI(mean, Stations[i].throughput, 0);
		printf("\n");
	}
}

// print the confidence interval given the sample mean, and an array of data (of size T)
// flag indicates if it is for the avg frames txd, or for the throughput
void printCI(double mean, double* array, int flag){
	double expectedSum = 0;
	for (int i = 0; i < T; i++){
		expectedSum = expectedSum + pow((array[i] - mean), 2);
	}

	double stdDev = sqrt(expectedSum/(T-1));

	double error = 2.776*(stdDev/pow(T,1/2));
	double c1 = mean - error;
	double c2 = mean + error;

	printf("%f %f\n", c1, c2);
	// write the throughput CI to file
	if (flag){
#ifdef OUTPUT
		fprintf(logfile, "%f %f\n", c1, c2);
#endif
	}
}

// verify the input
void checkInput(int argc, char* argv[]){
	if (argc < 11){
		printf("Warning: Not enough arguments to 'psim', please re-run with the right amount of arguments.\n");
		exit(0);
	}
}

