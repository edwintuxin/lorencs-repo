/* 
 * File:   main.c
 * Author: Mik
 *
 * Created on January 29, 2012, 1:44 PM
 */

#include "mainFunctions.h"
#include "child.h"
#include "memwatch.h"

/* Global variables */

int sleepTime;
char** processNames;			/* string array of process names to monitor */
char procToKill[MAX_PROC_NAME];	/* name of the current process to kill (to pass to child) */
int pidToKill;					/* pid of the current process to kill */
FILE *logfile;					/* pointer to log file*/
int procCount;			/* count of processes to monitor */

int main(int argc, char* argv[]) {
	pid_t pid = -1; 			/* variable to store the child's pid */
	pid_t *childpid; 			/* array to store all children's pid */
	int childCount = 0;			/* count of children */

	if (argc != 2){
		fprintf(stderr, "Error: Too few or too many arguments to %s.\n", argv[0]);
		exit(0);
	}

	logfile = fopen(getenv("PROCNANNYLOGS"), "w");
    readFile(argv[1]);			/* read in the config file */
    killPrevious(getpid()); 	/* kill previous instances of procnanny */
    
    // initialize the children and return an array of their PIDs
    childpid = initChildren(&pid, &childCount);

    if(pid == 0){
    	childExec(childpid); 	/* child code */
    }

    /* parent code (child has exited by now) */
    parentFinish(childpid, childCount);

    return (EXIT_SUCCESS);
}

/*
 *	    Things to test:
 * 		- ran processes named for ex "test.out", when config file specifies just "test"
 * 		- not including a number as the first line of config file
 * 		- invalid config file
 * 		- have the same process twice in the config file
 *
 * 		Tests done:
 * 		- ran 1129 copies of a process named 'one'.
 * 		- specific process names (eg. test vs. test.out)
 * 		- inclusing 'procnanny' in the config file as one of the processes to monitor
 * 		- run a config file with a 0 seconds wait time
 */
