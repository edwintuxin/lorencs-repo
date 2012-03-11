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
//int *nextSlot;				// array of station ids want to transmit in the next slot
int currSize;				// how many stations want to transmit in current slot
//int nextSize;				// how many stations want to transmit in next slot
int slot;

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
	//nextSlot = malloc(N*sizeof(int));
	//nextSize = 0;

	// run simulation T times
	for (int i = 0; i < T; i++){
		frameTx = 0;
		srand(t[i]);
		initStations();
		currSize = 0;
		//nextSize = 0;

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
	slot = 0;		// count of how many slot times have elapsed
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
					transmitFrame(i);
				}

				break;

			case 'P':

				currSize = 0;

				for (int j = 0; j < N; j++){
					// if station wants to transmit, and it hasn't already tried to previous transmit
					// then add it to the list of stations currently wanting to transmit
					if ((Stations[j].frameQ > 0) && (!Stations[j].tryingToTx)){
						currSlot[currSize] = j;
						currSize++;
					}

					//if the station has previously tried to transmit, add it with a prob of 1/N
					if ((Stations[j].tryingToTx) && (txNextSlot())){
						currSlot[currSize] = j;
						currSize++;
					}
				}

				//if collision, set all stations to tryingToTx
				if (currSize > 1){
					//printf("[slot %d] CLLSN \n", slot, i);
					for (int j = 0; j < currSize; j++){
						Stations[currSlot[j]].tryingToTx = 1;
					}
				// if no collision, transmit the one station that wants to transmit
				} else if (currSize == 1){
					transmitFrame(currSlot[0]);
					Stations[currSlot[0]].tryingToTx = 0;
				}

				break;

			case 'I':

				currSize = 0;

				for (int j = 0; j < N; j++){
					// if station wants to transmit, and it hasn't had a slot (1 to N) assigned to it
					// then add it to the list of stations currently wanting to transmit
					if ((Stations[j].frameQ > 0) && (Stations[j].tryingToTx < 1)){
						currSlot[currSize] = j;
						currSize++;
					}
				}

				//if collision, set slot offset for all stations trying to tx
				if (currSize > 1){
					//printf("[slot %d] CLLSN \n", slot, i);
					for (int j = 0; j < currSize; j++){
						Stations[currSlot[j]].tryingToTx = txSlotOffset(N);
					}
				// if no collision, transmit the one station that wants to transmit
				} else if (currSize == 1){
					transmitFrame(currSlot[0]);
					Stations[currSlot[0]].tryingToTx = -1;
				}

				//decrease 'tryingToTx' of all stations that have an offset
				for (int j = 0; j < N; j++){
					if (Stations[j].tryingToTx > 0){
						Stations[j].tryingToTx--;
					}
				}

				break;

			case 'B':

				currSize = 0;

				for (int j = 0; j < N; j++){
					// if station wants to transmit, and it hasn't had a slot (1 to N) assigned to it
					// then add it to the list of stations currently wanting to transmit
					if ((Stations[j].frameQ > 0) && (Stations[j].tryingToTx < 1)){
						currSlot[currSize] = j;
						currSize++;
					}
				}

				//if collision, set slot offset for all stations trying to tx
				if (currSize > 1){
					//printf("[slot %d] CLLSN \n", slot, i);
					for (int j = 0; j < currSize; j++){
						Stations[currSlot[j]].tryingToTx = txSlotOffset(pow(2, Stations[currSlot[j]].intExp));
						Stations[currSlot[j]].intExp++;
					}
				// if no collision, transmit the one station that wants to transmit
				} else if (currSize == 1){
					transmitFrame(currSlot[0]);
					Stations[currSlot[0]].tryingToTx = -1;
				}

				//decrease 'tryingToTx' of all stations that have an offset
				for (int j = 0; j < N; j++){
					if (Stations[j].tryingToTx > 0){
						Stations[j].tryingToTx--;
					}
				}

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
		if (Protocol == 'I'){
			Stations[i].tryingToTx = -1;
		} else {
			Stations[i].tryingToTx = 0;
		}
		Stations[i].intExp = 1;
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
			//printf("[slot %d] GENER [station %d] generate\n", slot, i);
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

// returns integer between 1 and upperBound
int txSlotOffset(int upperBound){
	double random = (double)rand() / (double)RAND_MAX;
	return 1 + floor(upperBound*random);
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

// transmits the frame of a station stationId
void transmitFrame (int stationId){
	frameTx++;
	Stations[stationId].frameTx++;
	//printf("[slot %d] TRANS [station %d] has txd. count: %d\n", slot, stationId, Stations[stationId].frameTx);
	Stations[stationId].frameQ--;

	frameList *frame = getLast(Stations[stationId].pendingFrames);

	if (Stations[stationId].frameTx > Stations[stationId].arraySize){
		Stations[stationId].arraySize = Stations[stationId].arraySize * 2;
		Stations[stationId].frameDelay = realloc(Stations[stationId].frameDelay, Stations[stationId].arraySize*sizeof(int));
	}

	Stations[stationId].frameDelay[Stations[stationId].frameTx-1] = frame->frameDelay;
	Stations[stationId].pendingFrames = deleteLast(Stations[stationId].pendingFrames);
}

/*void copyNextToCurr(){
	for (int i = 0; i < nextSize; i++){
		currSlot[i] = nextSlot[i];
	}
	currSize = nextSize;
	nextSize = 0;
}*/

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

		printf("|| ");
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

	printf("%f %f ", c1, c2);
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
	//free(nextSlot);
	free(Stations);
}

