/*
 * clientFunctions.h
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
#include "child.h"

#ifndef CLIENTFUNCTIONS_H_
#define CLIENTFUNCTIONS_H_
#endif /* CLIENTFUNCTIONS_H_ */

#define MAX_PROC_NAME 128
#define MAX_USER_NAME 15
#define SIGINT 2
#define SIGHUP 1

typedef struct {
	char name[MAX_PROC_NAME];
	int sleep;
} procs;

typedef struct {
	int m_pid;			/* process ID of the process the child is monitoring (0 if idle) */
	int p2c[2];			/* pipes to communicate from parent to child */
	int c2p[2];			/* pipes to communicate from child to parent */
} child;

void initChildren(int *pid, int *_c2p, int *_p2c);
void parentLoop();
void readChildMessages();
void rescanProcs();
int getIdleChildIndex();
int processMonitored(int pid);
void waitForChildren();
void cleanup();
int exists(char* line, int arraySize);
void resetMsg();
void killPrevious(char* procname, int parentID);
int* getPidList(char* procName,int *arraySize);
void connectToServer();

// Non-C99 compliant function prototypes
FILE *popen(const char *command, const char *type);
int pclose(FILE *stream);
int getpid();
int strnlen();
int kill(pid_t pid, int sig);
int isalpha(int character);
int getdtablesize();
void bzero();
void bcopy();
