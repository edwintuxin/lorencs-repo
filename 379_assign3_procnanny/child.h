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

void childExec(int *c2p, int *p2c);
void childExit();
void monitorProcess(char *procToKill, char *pidToKill, int sleepTime, int *c2p, int *p2c);
void timestampToParent(char* input, int *c2p);
