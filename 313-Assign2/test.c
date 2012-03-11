/*
 * test.c
 *
 *  Created on: Mar 10, 2012
 *      Author: Mik
 */
#include "frame.h"
int N = 20;

int txNextSlot(){
		double p = 1/(double)N;
		double random = (double)rand() / (double)RAND_MAX;
		if (random < p){
			return 1;
		}
		return 0;
	}

int main(int argc, char * argv[]){
	int seed;
	sscanf(argv[1], "%d", &seed);
	srand(seed);

	int txCount = 0;

	for (int i = 0; i < 100; i++){
		if (txNextSlot()){
			txCount++;
		}
	}

	printf("txCount: %d\n", txCount);

	return 0;
}
