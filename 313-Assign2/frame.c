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

//returns the last element of the list
frameList* getLast (frameList *first){
	// special case for the list containing a single element
	if( first->next == NULL ) {
		return first;
	}

	frameList *temp = first;
	while( temp->next->next != NULL ) {
	temp = temp->next;
	}

	return temp->next;
}

//deletes the first frame in the frameList
void deleteLast (frameList *first){
	// special case for an empty list
	if( first == NULL ) {
	return;
	}

	// special case for the list containing a single element
	if( first->next == NULL ) {
		first = NULL;
		return;
	}

	// iterate through list, starting at root
	frameList *temp = first;

	// when current->next is null, current is the last element of this list and
	// last will become the new last element
	while( temp->next->next != NULL ) {
	temp = temp->next;
	}

	free(temp->next);

	// cut off current
	temp->next = NULL;

	return;
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

