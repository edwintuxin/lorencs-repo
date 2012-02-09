/*
 * child.c
 *
 *  Created on: Feb 8, 2012
 *      Author: lorencs
 *
 *  Contains the code that is to be executed by the child
 */

#include "mainFunctions.h"
#include "memwatch.h"

/* global vars from main */
extern int sleepTime;					/* time to sleep for */
extern char procToKill[MAX_PROC_NAME];	/* name of the current process to kill */
extern int pidToKill;					/* pid of the current process to kill */
extern FILE *logfile;					/* pointer to log file*/

void childExec(pid_t *childpid){
	int retval;			/* child's return value */
	int killResult;		/* boolean to hold the return value of the kill funct */
	char string[20];	/* string to convert integers into string*/
	int *pidList;
	int pidCount;

	sprintf(string, "%d", pidToKill);

	// print message to logfile
	char message[100] = "Info: Initializing monitoring of process '";
	strcat(message, procToKill);
	strcat(message, "' (PID ");
	strcat(message, string);
	strcat(message, ").\n");
	timestamp(message);

	//go to sleep
	sleep(sleepTime);

	// use getPidList function on the PID of the process this child is monitoring
	// to see if it has exited before the wait time
	pidList = getPidList(string, &pidCount);

	if (pidCount == 0){
		// if the process has exited while child was sleeping
		sprintf(string, "%d", pidToKill);
		strcpy(message,"Info: PID ");
		strcat(message, string);
		strcat(message, " (");
		strcat(message, procToKill);
		strcat(message, ") has exited before exceeding ");
		sprintf(string, "%d", sleepTime);
		strcat(message, string);
		strcat(message, " seconds.\n");
		timestamp(message);
		retval = 0;
	} else {
		sprintf(string, "%d", pidToKill);
		// kill the process
		killResult = kill(pidToKill, SIGKILL);

		// if kill successful, print success message to log
		if (killResult == 0){
			strcpy(message,"Action: PID ");
			strcat(message, string);
			strcat(message, " (");
			strcat(message, procToKill);
			strcat(message, ") killed after exceeding ");
			sprintf(string, "%d", sleepTime);
			strcat(message, string);
			strcat(message, " seconds.\n");
			retval = 1;
		// if kill unsuccessful, print fail message
		} else if (killResult == -1){
			strcpy(message, "Error: Unable to kill PID ");
			strcat(message, string);
			strcat(message, " (");
			strcat(message, procToKill);
			strcat(message, ").\n");
			retval = 0;
		}
		timestamp(message);
	}

	free(pidList);
	cleanup(childpid, NULL);
	exit(retval);
}
