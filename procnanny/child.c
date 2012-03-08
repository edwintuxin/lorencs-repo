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

void childExec(int *child_pipes){
	while(1){

		char message[256];
		memset(message, 0, 256);
		msg = read_message(child_pipes[0]);
		strcpy(message, msg->body);
		resetMsg();

		if (!strcmp(message, "exit")){
			childExit();
		} else {
			char procToKill[MAX_PROC_NAME];
			char pidToKill[20];
			int sleepTime;

			sscanf(message, "%s %s %d", procToKill, pidToKill, &sleepTime);
			monitorProcess(procToKill, pidToKill, sleepTime, child_pipes);
		}

	}
}

void childExit(){
	cleanup(NULL);
	exit(1);
}

void monitorProcess(char *procToKill, char *pidToKill, int sleepTime, int *child_pipes){
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
		timestamp(output);
	}

	// send message to parent that child is available, followed by
	// integer indicating if kill occurred or not
	char msgStr[20] = "available ";
	char msgValStr[5];
	sprintf(msgValStr, "%d", msgVal);
	strcat(msgStr, msgValStr);
	msg = init_message(msgStr);
	write_message(child_pipes[1],msg);
	resetMsg();

	free(pidList);
}
