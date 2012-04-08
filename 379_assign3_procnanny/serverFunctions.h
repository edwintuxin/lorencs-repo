/*
 * serverFunctions.h
 *
 *  Created on: Apr 7, 2012
 *      Author: Mik
 */

#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <signal.h>
#include <setjmp.h>
#include <unistd.h>
#include <time.h>
#include <string.h>
#include <termios.h>
#include <sys/time.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/select.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include "memwatch.h"
#include "pipeMessage.h"



#ifndef SERVERFUNCTIONS_H_
#define SERVERFUNCTIONS_H_
#endif /* SERVERFUNCTIONS_H_ */

#define MAX_PROC_NAME 128
#define MAX_USER_NAME 15
#define SIGINT 2
#define SIGHUP 1

typedef struct {
	char name[MAX_PROC_NAME];
	int sleep;
} procs;

void readFile();
void serverLoop();
void signalHandler(int signalNum);
void cleanup();
void setHandler(int sigType, void* handler, int i);
void printToFile(char* input, int p2stdout, FILE* file);
int exists(char* line, int arraySize);
void resetMsg();
void killPrevious(char* procname, int parentID);
int* getPidList(char* procName,int *arraySize);
void sendConfig(int client);

// Non-C99 compliant function prototypes
FILE *popen(const char *command, const char *type);
int pclose(FILE *stream);
int getpid();
int strnlen();
int kill(pid_t pid, int sig);
int isalpha(int character);
int getdtablesize();
void outputStartMsg();
