/*
 * File:   main.c
 * Author: lorencs
 *
 * Created on January 29, 2012, 1:44 PM
 */

#include "mainFunctions.h"
#include "memwatch.h"

/* Global variables */
int sleepTime;					/* amount of time to sleep to pass to child */
char configPath[128];			/* path to the config file */
FILE *logfile;					/* pointer to log file */
int procCount;					/* count of processes to monitor */
procs monitorProcs[128];		/* array of structs */
child *childPool;				/* pool of all children */
int poolSize;					/* size of currently malloced pool */
int childCount;					/* current count of children in pool */
int idleChildCount;				/* count of any idle children */
struct pipeMessage* msg;		/* pointer for the pipe messages */
int killCount;					/* kill count that parent keeps track of */
struct sigaction* newAction[2];	/* action struct to use with signal handler */

int main(int argc, char* argv[]) {
	pid_t pid = -1; 			/* variable to store the child's pid */
	childCount = 0;
	idleChildCount = 0;
	killCount = 0;
	int c2p[2];			/* the set of pipes that will be passed to each child */
	int p2c[2];

	// setup signal handlers
	setHandler(SIGINT, signalHandler, 0);
	setHandler(SIGHUP, signalHandler, 1);

	if (argc != 2){
		fprintf(stderr, "Error: Too few or too many arguments to %s.\n", argv[0]);
		exit(0);
	}
	strncpy(configPath, argv[1], strlen(argv[1]));

	logfile = fopen(getenv("PROCNANNYLOGS"), "w");

    readFile();					/* read in the config file */
    killPrevious(getpid()); 	/* kill previous instances of procnanny */

    // initialize the children
    initChildren(&pid, c2p, p2c);

    if(pid == 0){
    	childExec(c2p, p2c); 	// child code
    }

    // parent loops forever until it is sent SIGINT
    parentLoop();

    /* shouldn't get here */
    return (EXIT_SUCCESS);
}
