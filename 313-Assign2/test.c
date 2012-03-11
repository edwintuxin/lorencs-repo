/*
 * test.c
 *
 *  Created on: Mar 10, 2012
 *      Author: Mik
 */
#include "frame.h"


int main(){

	frameList *first = NULL;
	increaseDelay(first);

	first = addFrame(first);

	frameList* curr, * prev;
	curr = first;
	prev = NULL;

	while( curr != NULL ){
		prev = curr;
		curr = curr->next;
		printf("delay: %s\n", prev->frameDelay);
	}

	return 0;
}
