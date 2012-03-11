/*
 * test.c
 *
 *  Created on: Mar 10, 2012
 *      Author: Mik
 */
#import "frame.h"

int N = 20;

// returns integer between 1 and N
int txNextSlotInt(){
	double random = (double)rand() / (double)RAND_MAX;
	return 1 + floor(N*random);
}

int main(int argc, char * argv[]){
	int seed;
	sscanf(argv[1], "%d", &seed);
	srand(seed);

	int txCount = 0;

	for (int i = 0; i < 100; i++){
		printf("%f\n", txNextSlotInt());
	}

	printf("txCount: %d\n", txCount);

	return 0;
}
