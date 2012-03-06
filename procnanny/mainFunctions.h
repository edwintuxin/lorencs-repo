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
#include <unistd.h>
#include <time.h>
#include <sys/time.h>
#include <sys/types.h>
#include <sys/wait.h>
#include "memwatch.h"

#define MAX_PROC_NAME 128
#define MAX_USER_NAME 15

#ifndef MAINFUNCTIONS_H_
#define MAINFUNCTIONS_H_


#endif /* MAINFUNCTIONS_H_ */

typedef struct {
	char name[MAX_PROC_NAME];
	int sleep;
} procs;

typedef struct {
	int childId;
	int pid;
	int fd[2];
	int busy;
} child;

void readFile(char* config);
void killPrevious(int parentID);
void killProcess(char* procName);
void timestamp(char* input);
int* getPidList(char* procName, int *arraySize);
void cleanup(int *childpid, int *status);
pid_t* initChildren(int *pid, int *childCount);
void parentFinish(pid_t* childpid, int childCount);
int exists(char* line, int arraySize);

// Non-C99 compliant function prototypes
FILE *popen(const char *command, const char *type);
int pclose(FILE *stream);
int getpid();
int strnlen();
int kill(pid_t pid, int sig);
int isalpha(int character);
