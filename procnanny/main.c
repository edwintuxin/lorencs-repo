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

int main(int argc, char* argv[]) {
	pid_t pid = -1; 			/* variable to store the child's pid */
	int childCount = 0;			/* count of children */

	if (argc != 2){
		fprintf(stderr, "Error: Too few or too many arguments to %s.\n", argv[0]);
		exit(0);
	}

	logfile = fopen(getenv("PROCNANNYLOGS"), "w");
	 if (logfile == NULL) {
	        printf("Error");
	    }


    readFile(argv[1]);			/* read in the config file */
    killPrevious(getpid()); 	/* kill previous instances of procnanny */

    // initialize the children and return an array of their PIDs
    initChildren(&pid, &childCount);

    if(pid == 0){
    	childExec(childpid); 	// child code
    }

    // parent code (child has exited by now)
    parentFinish(childpid, childCount);

    for (int i = 0; i < procCount; i++){
    	printf("Name: %s\n", monitorProcs[i].name);
    	printf("Sleeptime: %d\n", monitorProcs[i].sleep);
    }

    return (EXIT_SUCCESS);
}

/*
 *
 */
