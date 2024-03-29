/*
 * frame.h
 *
 *  Created on: Mar 10, 2012
 *      Author: lorencs
 */

#include <stdio.h>
#include <time.h>
#include <string.h>
#include <stdlib.h>
#include <ctype.h>
#include <math.h>
#include "memwatch.h"

#define MAX_MNAME 128

//matrix list structure
typedef struct frameList{
    int frameDelay;
    struct frameList *next;
} frameList;

//ADT function prototypes
frameList* addFrame(frameList *first);
frameList* deleteLast (frameList *first);
frameList* getLast (frameList *first);
void freeMemory(frameList *first);
void printNodes(frameList *first);
void increaseDelay(frameList *first);
