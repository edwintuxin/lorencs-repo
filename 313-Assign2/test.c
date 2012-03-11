/*
 * test.c
 *
 *  Created on: Mar 10, 2012
 *      Author: Mik
 */
#include "frame.h"


int txNextSlot(){
		double p = 1/(double)N;
		double random = (double)rand() / (double)RAND_MAX;
		if (random < p){
			return 1;
		}
		return 0;
	}

int main(){

	int N = 20;



	for (int i = 0; i < 100; i++){
		if (txNextSlot()){
			printf("transmit\n");
		} else {
			printf("no transmit\n");
		}
	}
}
