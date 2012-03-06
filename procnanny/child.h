/*
 * child.h
 *
 *  Created on: Feb 8, 2012
 *      Author: lorencs
 */

#include "memwatch.h"

#ifndef CHILD_H_
#define CHILD_H_


#endif /* CHILD_H_ */

void childExec(child *childPool, int childId, int *child_read);
void getdtablesize();
void childExit(child *childPool);
void monitorProcess(char *procToKill, char *pidToKill, int sleepTime);
