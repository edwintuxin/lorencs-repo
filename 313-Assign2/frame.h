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
typedef struct {
    int frameDelay;
    frameList *next;
} frameList;

//ADT function prototypes
frameList *addMatrix(frameList *first);
frameList *deleteMatrix (frameList *first);
void freeMemory(frameList *first);
void increaseDelay(frameList *first);
