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

void childExec(int *child_pipes);
void childExit();
void monitorProcess(char *procToKill, char *pidToKill, int sleepTime, int *child_pipes);
