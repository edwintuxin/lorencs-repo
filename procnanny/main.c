/* 
 * File:   main.c
 * Author: lorencs
 *
 * Created on January 29, 2012, 1:44 PM
 */

#include "mainFunctions.h"
#include "child.h"
#include "memwatch.h"

/* Global variables */
int sleepTime;					/* amount of time to sleep to pass to child */
char procToKill[MAX_PROC_NAME];	/* name of the current process to kill (to pass to child) */
int pidToKill;					/* pid of the current process to kill */
FILE *logfile;					/* pointer to log file */
int procCount;					/* count of processes to monitor */
procs monitorProcs[128];		/* array of structs */
child *childPool;				/* pool of all children */
int poolSize;					/* size of currently malloced pool */
int childCount;					/* current count of children in pool */

int main(int argc, char* argv[]) {
	pid_t pid = -1; 			/* variable to store the child's pid */
	childCount = 0;				/* count of children */
	int child_pipes[2];			/* the set of pipes that will be passed to each child */

	if (argc != 2){
		fprintf(stderr, "Error: Too few or too many arguments to %s.\n", argv[0]);
		exit(0);
	}

	logfile = fopen(getenv("PROCNANNYLOGS"), "w");

    readFile(argv[1]);			/* read in the config file */
    killPrevious(getpid()); 	/* kill previous instances of procnanny */

    // initialize the children
    initChildren(&pid, child_pipes);

    if(pid == 0){
    	childExec(childPool,childCount, child_pipes); 	// child code
    }

    // parent code (child has exited by now)
    parentFinish(childPool);

    return (EXIT_SUCCESS);
}

/*
 * fix tghe bufgfering of messages
 */
