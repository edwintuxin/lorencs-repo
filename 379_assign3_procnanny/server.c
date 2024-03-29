/* 
 * File:   main.c
 * Author: lorencs
 *
 * Created on January 29, 2012, 1:44 PM
 */

#include "serverFunctions.h"
#include "memwatch.h"

/* Global variables */
char configPath[128];			/* path to the config file */
FILE *logfile;					/* pointer to log file */
FILE *serverlog;				/* pointer to server log file */
int procCount;					/* count of processes to monitor */
procs monitorProcs[128];		/* array of structs */
struct pipeMessage* msg;		/* pointer for the pipe messages */
struct sigaction* newAction[2];	/* action struct to use with signal handler */
int clientCount;				/* count of currently connected clients */
int sock;						/* socket on which the server will listen for clients */
int clients[32];				/* array of sockets to a max of 32 clients */
int serverPort;					/* port that the server will attempt to listen on */
int killCount;					/* kill count of all clients (calc'd at the end) */

int main(int argc, char* argv[]) {
	// setup signal handlers
	setHandler(SIGINT, signalHandler, 0);
	setHandler(SIGHUP, signalHandler, 1);

	//initialize port and counts
	serverPort = 2000;
	clientCount = 0;
	killCount = 0;

	// check arg count and scan in arg
	if (argc != 2){
		fprintf(stderr, "Error: Too few or too many arguments to %s.\n", argv[0]);
		cleanup();
		exit(0);
	}
	strncpy(configPath, argv[1], strlen(argv[1]));

	// open the log files
	serverlog = fopen(getenv("PROCNANNYSERVERINFO"), "w");
	logfile = fopen(getenv("PROCNANNYLOGS"), "w");

    readFile();										/* read in the config file */
    killPrevious("procnanny.server", getpid()); 	/* kill previous instances of procnanny server */

    // server loops forever until it is sent SIGINT
    serverLoop();

    /* shouldn't get here */
    return (EXIT_SUCCESS);
}
