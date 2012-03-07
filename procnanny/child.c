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
#include "child.h"

/* global vars from main */
struct pipeMessage* msg;
extern FILE *logfile;					/* pointer to log file*/
int retval;								/* child's return value */

void childExec(child *childPool, int childId, int *child_pipes){
	while(1){

		char message[256];
		memset(message, 0, 256);
		msg = read_message(child_pipes[0]);
		strcpy(message, msg->body);
		free(msg->body);
		free(msg);

		if (!strcmp(message, "exit")){
			childExit(childPool);
		} else {
			char procToKill[MAX_PROC_NAME];
			char pidToKill[20];
			int sleepTime;

			sscanf(message, "%s %s %d", procToKill, pidToKill, &sleepTime);
			monitorProcess(procToKill, pidToKill, sleepTime);
		}

	}
}

void childExit(child *childPool){
	cleanup(childPool, NULL);
	exit(retval);
}

void monitorProcess(char *procToKill, char *pidToKill, int sleepTime){
	int killResult;		/* boolean to hold the return value of the kill funct */
	int *pidList;
	int pidCount;
	char string[10];

	// print message to logfile
	char output[100] = "Info: Initializing monitoring of process '";
	strcat(output, procToKill);
	strcat(output, "' (PID ");
	strcat(output, pidToKill);
	strcat(output, ").\n");
	timestamp(output);

	//go to sleep
	sleep(sleepTime);

	// use getPidList function on the PID of the process this child is monitoring
	// to see if it has exited before the wait time
	pidList = getPidList(pidToKill, &pidCount);

	if (pidCount == 0){
		// if the process has exited while child was sleeping
		strcpy(output,"Info: PID ");
		strcat(output, pidToKill);
		strcat(output, " (");
		strcat(output, procToKill);
		strcat(output, ") has exited before exceeding ");
		sprintf(string, "%d", sleepTime);
		strcat(output, string);
		strcat(output, " seconds.\n");
		timestamp(output);
		retval = 0;
	} else {
		int pidToKillInt;
		sscanf(pidToKill, "%d", &pidToKillInt);
		// kill the process
		killResult = kill(pidToKillInt, SIGKILL);

		// if kill successful, print success message to log
		if (killResult == 0){
			strcpy(output,"Action: PID ");
			strcat(output, pidToKill);
			strcat(output, " (");
			strcat(output, procToKill);
			strcat(output, ") killed after exceeding ");
			sprintf(string, "%d", sleepTime);
			strcat(output, string);
			strcat(output, " seconds.\n");
			retval = 1;
		// if kill unsuccessful, print fail message
		} else if (killResult == -1){
			strcpy(output, "Error: Unable to kill PID ");
			strcat(output, pidToKill);
			strcat(output, " (");
			strcat(output, procToKill);
			strcat(output, ").\n");
			retval = 0;
		}
		timestamp(output);
	}

	free(pidList);
}
