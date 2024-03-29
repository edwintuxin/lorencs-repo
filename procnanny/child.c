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

struct pipeMessage* msg;
int msgVal;								/* child's return value */

void childExec(int *c2p, int *p2c){
	// ignore interrupts and sighups as the parent should handle those
	signal(SIGINT, SIG_IGN);
	signal(SIGHUP, SIG_IGN);

	while(1){

		char message[256];
		memset(message, 0, 256);
		msg = read_message(p2c[0]);
		strcpy(message, msg->body);
		resetMsg();

		//printf("message is: %s\n", message);
		if (!strcmp(message, "exit")){
			childExit(c2p);
		} else if (!strncmp(message, "monitor", 7)){
			char procToKill[MAX_PROC_NAME];
			char pidToKill[20];
			int sleepTime;

			sscanf(message, "monitor %s %s %d", procToKill, pidToKill, &sleepTime);
			monitorProcess(procToKill, pidToKill, sleepTime, c2p, p2c);
		}

	}
}

// cleanup and inform parent that chidl is exiting
void childExit(int *c2p){
	msg = init_message("exit complete");
	write_message(c2p[1],msg);
	resetMsg();
	cleanup(NULL);

	exit(EXIT_SUCCESS);
}

void monitorProcess(char *procToKill, char *pidToKill, int sleepTime, int *c2p, int *p2c){
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
	timestamp(output, 0);

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
		timestamp(output, 0);
		msgVal = 0;
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
			msgVal = 1;
		// if kill unsuccessful, print fail message
		} else if (killResult == -1){
			strcpy(output, "Error: Unable to kill PID ");
			strcat(output, pidToKill);
			strcat(output, " (");
			strcat(output, procToKill);
			strcat(output, ").\n");
			msgVal = 0;
		}
		timestamp(output, 0);
	}

	// send message to parent that child is available, followed by
	// integer indicating if kill occurred or not
	char msgStr[20] = "available ";
	char msgValStr[5];
	sprintf(msgValStr, "%d", msgVal);
	strcat(msgStr, msgValStr);
	msg = init_message(msgStr);
	write_message(c2p[1],msg);
	resetMsg();

	free(pidList);
}
