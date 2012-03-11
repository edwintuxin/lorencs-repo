/*
 * test.c
 *
 *  Created on: Mar 10, 2012
 *      Author: Mik
 */
#include "frame.h"


int main(){

	frameList *first = NULL;

	first = addFrame(first);
	increaseDelay(first);
	first = addFrame(first);
	first = addFrame(first);
	first = addFrame(first);

	increaseDelay(first);

	first = addFrame(first);
	increaseDelay(first);
	first = addFrame(first);

	frameList *lastnode = getLast(first);

	frameList *curr, *prev;
	curr = first;
	prev = NULL;

	while( curr != NULL ){
		prev = curr;
		printf("delay: %d\n", prev->frameDelay);
		curr = curr->next;
	}


	printf("last is : %d\n",lastnode->frameDelay);
	freeMemory(first);

	return 0;
}
