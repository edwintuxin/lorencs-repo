/*
 * psim.c
 *
 *  Created on: Mar 10, 2012
 *      Author: lorencs
 */

#include "psim.h"
#include "memwatch.h"

#define DEBUG

/* input global variables */
char Protocol;				// type of protocol
int N;						// number of stations
float p;					// probability of frame generation
int R;						// number of slots to simulate
int T;						// number of trials
int t[5];					// array of 5 seeds for 5 trials

/* other global vars */
station *Stations;
double throughput[5];		// array of throughputs for each trial
int frameTx;				// # of successful transmissions (in one trial)

// used in Protocol P
int *currSlot;			// array of station ids want to transmit in the current slot
int *nextSlot;				// array of station ids want to transmit in the next slot
int currSize;				// how many stations want to transmit in current slot
int nextSize;				// how many stations want to transmit in next slot

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
	currSlot = malloc(N*sizeof(int));
	nextSlot = malloc(N*sizeof(int));
	currSize = 0;
	nextSize = 0;

	// run simulation T times
	for (int i = 0; i < T; i++){
		frameTx = 0;
		srand(t[i]);
		initStations();
		currSize = 0;
		nextSize = 0;

		runSim();

		throughput[i] = (double)frameTx/ (double)R;

		for (int j = 0; j < N; j++){
			Stations[j].throughput[i] = (double)Stations[j].frameTx/ (double)R;
			Stations[j].avgDelay[i] = getAvgDelay(Stations[j].frameDelay, Stations[j].frameTx);
			free(Stations[j].frameDelay);
		}
	}

	printStats(argc, argv);
	cleanup();

	return 0;
}


void runSim(){
	int slot = 0;		// count of how many slot times have elapsed
	int i = 0;			// int form 0 to N-1 showing which slot of the bus it is in
	while (slot < R){
		if (i == N){
			i = 0;
		}

		generateFrames();

		for (int j = 0; j < N; j++){
			increaseDelay(Stations[j].pendingFrames);
		}

		// behave according to the specified protocol
		switch(Protocol){
			case 'T':
				// if station corresponding to slot has frame(s) to tx
				// transmit one frame
				if (Stations[i].frameQ > 0){
					frameTx++;
					Stations[i].frameTx++;
					Stations[i].frameQ--;

					frameList *frame = getLast(Stations[i].pendingFrames);

					if (Stations[i].frameTx > Stations[i].arraySize){
						Stations[i].arraySize = Stations[i].arraySize * 2;
						Stations[i].frameDelay = realloc(Stations[i].frameDelay, Stations[i].arraySize*sizeof(int));
					}

					Stations[i].frameDelay[Stations[i].frameTx-1] = frame->frameDelay;
					Stations[i].pendingFrames = deleteLast(Stations[i].pendingFrames);
				}


				break;

			case 'P':
				copyNextToCurr();	// copy the stations from "nextSlot" to "currSlot"

				for (int j = 0; j < N; j++){
					// if station wants to transmit, and it isn't already wanting to transmit this slot
					// and if it isn't trying to pick a slot to tx in, add it to list of stations wanting to transmit
					if ((Stations[j].frameQ > 0) && (!isIn(j, currSlot, currSize)) && (!Stations[j].tryingToTx)){
						currSlot[currSize] = j;
						currSize++;
					}
				}

				//if collision, set the stations to tryingToTx, and choose next slot with prob 1/N
				if (currSize > 1){
					for (int j = 0; j < currSize; j++){
						Stations[currSlot[j]].tryingToTx = 1;
						if (txNextSlot()){
							nextSlot[nextSize] = j;
							nextSize++;
						}
					}
				}

				break;

			case 'I':

				break;

			case 'B':

				break;
		}

		i++;
		slot++;

	}

}

// resets/initializes  all stations
void initStations(){
	for (int i = 0; i < N; i++){
		Stations[i].frameQ = 0;
		Stations[i].frameTx = 0;
		Stations[i].tryingToTx = 0;
		Stations[i].arraySize = 100;
		Stations[i].pendingFrames = NULL;
		Stations[i].frameDelay = malloc(Stations[i].arraySize*sizeof(int));
	}
}

// generates frames for all N stations based on probability p
void generateFrames(){
	for (int i = 0; i < N; i++){
		double random = (double)rand() / (double)RAND_MAX;
		if (random < p){
			Stations[i].frameQ++;
			Stations[i].pendingFrames = addFrame(Stations[i].pendingFrames);
		}
	}
}

// returns 1 with a probability of 1/N
int txNextSlot(){
	double p = 1/(double)N;
	double random = (double)rand() / (double)RAND_MAX;
	if (random < p){
		return 1;
	}
	return 0;
}

double getAvgDelay(int *array, int size){
	int sum = 0;
	for (int i = 0; i < size; i++){
		sum = sum + array[i];
	}

	double avg = (double)sum/(double)size;

	return avg;
}

int isIn (int num, int *array, int size){
	for (int i = 0; i < size; i++){
		if (num == array[i]){
			return 1;
		}
	}
	return 0;
}

void copyNextToCurr(){
	for (int i = 0; i < nextSize; i++){
		currSlot[i] = nextSlot[i];
	}
	currSize = nextSize;
	nextSize = 0;
}

// print out the end of execution statistics
void printStats(int argc, char* argv[]){
	for (int i = 1; i < 6; i++){
		printf("%s ", argv[i]);
	}
	printf("\n");

	// calculate mean of throughput
	double mean = 0;
	for (int i = 0; i < T; i++){
		mean = mean + throughput[i];
	}
	mean = mean/T;


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

		// calculate mean of avg delays
		mean = 0;
		for (int j = 0; j < T; j++){
			mean = mean + Stations[i].avgDelay[j];
		}
		mean = mean/T;

		printf("%f ", mean);
		printCI(mean, Stations[i].avgDelay, 0);

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

#ifdef OUTPUT
		fprintf(logfile, "%f %f\n", c1, c2);
#endif

}

// verify the input
void checkInput(int argc, char* argv[]){
	if (argc < 11){
		printf("Warning: Not enough arguments to 'psim', please re-run with the right amount of arguments.\n");
		exit(0);
	}
}

void cleanup(){
	for (int i = 0; i < N; i++){
		freeMemory(Stations[i].pendingFrames);
	}

	free(currSlot);
	free(nextSlot);
	free(Stations);
}

