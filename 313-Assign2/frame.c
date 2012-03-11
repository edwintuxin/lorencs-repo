/*
 * frame.c
 *
 *  Created on: Mar 10, 2012
 *      Author: lorencs
 */

#include "frame.h"
#include "memwatch.h"


//creates new frame, sets its delay to 1, and increases delay
//of all the frames already in the list
frameList* addFrame(frameList *first){
	frameList *newnode;
    newnode = malloc(sizeof(frameList));
    newnode->frameDelay = 0;
    newnode->next = first;
    return newnode;
}

//deletes the first frame in the frameList
frameList* deleteLast (frameList *first){
    //find matrix
	frameList *curr, *prev;

	while( curr->next != NULL ){
			prev = curr;
			curr = curr->next;
		}

	prev->next = NULL;
    //free space occupied by the node
    free(first);

    return prev;
}

void increaseDelay(frameList *first){
	frameList* curr, * prev;
	curr = first;
	prev = NULL;

	while( curr != NULL ){
		prev = curr;
		curr = curr->next;
		prev->frameDelay++;
	}
}

//free the memory allocated to all the frames
void freeMemory(frameList *first){
	frameList* curr, * prev;
    curr = first;
    prev = NULL;

    while( curr != NULL ){
        prev = curr;
        curr = curr->next;
        free(prev);
    }
}

