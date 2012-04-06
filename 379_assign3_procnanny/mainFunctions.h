/*
 * mainFunctions.h
 *
 *  Created on: Jan 31, 2012
 *      Author: lorencs
 */

#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <signal.h>
#include <setjmp.h>
#include <unistd.h>
#include <time.h>
#include <sys/time.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/select.h>
#include "memwatch.h"
#include "pipeMessage.h"
#include "child.h"

#define MAX_PROC_NAME 128
#define MAX_USER_NAME 15
//#define SIGKILL 2

#ifndef MAINFUNCTIONS_H_
#define MAINFUNCTIONS_H_


#endif /* MAINFUNCTIONS_H_ */

typedef struct {
	char name[MAX_PROC_NAME];
	int sleep;
} procs;

typedef struct {
	int m_pid;			/* process ID of the process the child is monitoring (0 if idle) */
	int p2c[2];			/* pipes to communicate from parent to child */
	int c2p[2];			/* pipes to communicate from child to parent */
} child;

void readFile();
void killPrevious(int parentID);
void killProcess(char* procName);
void timestamp(char* input, int p2stdout);
int* getPidList(char* procName, int *arraySize);
void cleanup();
void initChildren(int *pid, int *c2p, int *p2c);
void parentLoop();
void readChildMessages();
void rescanProcs();
int getIdleChildIndex();
int processMonitored(int pid);
void signalHandler(int signalNum);
void setHandler(int sigType, void* handler, int i);
void waitForChildren();
int exists(char* line, int arraySize);
void resetMsg();

// Non-C99 compliant function prototypes
FILE *popen(const char *command, const char *type);
int pclose(FILE *stream);
int getpid();
int strnlen();
int kill(pid_t pid, int sig);
int isalpha(int character);
int getdtablesize();
