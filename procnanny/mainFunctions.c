#include "mainFunctions.h"
#include "memwatch.h"

//global variables from main
extern int sleepTime;
extern char procToKill[MAX_PROC_NAME];
extern int pidToKill;
extern int procCount;
extern FILE* logfile;
extern procs monitorProcs[128];

//reads the config file and stores the info from it
void readFile(char* config){
	//load file
	FILE *configFile = fopen(config, "r");

	//checks if file exists
	if (!(configFile)){
		fprintf(stderr, "Error: The specified .config file does not exist.\n");
		exit(0);
	}

	//scan in process names
	char line[MAX_PROC_NAME+1];
	int lineCount = 0;

	// while config file has lines
	while (fgets(line, MAX_PROC_NAME+1, configFile) != NULL) {
		char message[150];
		char name[MAX_PROC_NAME];
		int sleeptime;

		sscanf(line, "%s %d", name, &sleeptime);

		// warn user that monitoring 'procnanny' will cause unexpected behavior and exit
		if (!strcmp(name, "procnanny")){
			strcpy(message, "Warning: You are attempting to monitor the "
					"'procnanny' process. Exiting to avoid unexpected behavior.\n");
			timestamp(message);

			procCount = lineCount;
			cleanup(NULL, NULL);
			exit(0);
		}



		// check if the process name has been already scanned in, ignore it if it has
		if (exists(name, lineCount)){
			strcpy(message, "Warning: Ignoring duplicate process '");
			strcat(message, name);
			strcat(message, "' in the .config file.\n");
			timestamp(message);
			continue;
		}

		lineCount++;

		strcpy(monitorProcs[lineCount-1].name, name);
		monitorProcs[lineCount-1].sleep = sleeptime;
	}

	//set process count
	procCount = lineCount;

	fclose(configFile);
}

// returns true if the string 'line' exists in the processNames array
// arraySize is passed because the check is done as the array is being
// dynamically allocated
int exists(char* line, int arraySize){
	int returnVal = 0;

	for (int i = 0; i < arraySize; i++){
		if (!strcmp(line, monitorProcs[i].name)){
			returnVal = 1;
			break;
		}
	}

	return returnVal;
}

// kills all previous instances of procnanny
void killPrevious(int parentID){
	int lineCount = 0;
	int killcount = 0;

	// get a (dynamically alloc'd) list of PIDs with the proc name 'procnanny'
	// save the count of these PIDs to lineCount
	int *pidList = getPidList("procnanny", &lineCount);

	// if no previous processes found (if count is 1, the only procnanny is the one running)
	if (lineCount < 2){
		free(pidList);
		return;
	}

	// kill the processes and count how many were killed
	for(int i=0; i < lineCount; i++){
		if (pidList[i] != parentID){
			kill(pidList[i], SIGKILL);
			killcount++;
		}
	}

	// check if there are still other procnanny's left
	free(pidList);
	pidList = getPidList("procnanny", &lineCount);

	char output[1000] = "";
	char kc[10];

	// if more than 1 'procnanny' running, kill was unsuccessful
	if (lineCount > 2){
		strcat(output, "Info: Unable to clean up ");
		sprintf(kc,"%d", lineCount - 1);
		strcat(output, kc);
	// else print successful clean message
	} else {
		strcat(output, "Action: Cleaned up ");
		sprintf(kc,"%d",killcount);
		strcat(output, kc);
	}
	strcat(output, " previous 'procnanny' process(es).\n");
	timestamp(output);

	free(pidList);
}

// initializes all the children, returns the array of child PIDs
// take pointer to pid (main needs the modified value of it) and pointer to childCount
initChildren(int *pid, int *childCount){
    int *pidList;
    int pidCount;
    int arraySize = procCount;

    // dynamic array to hold the pids of children
	childPool = malloc(procCount*sizeof(child));

    // for each process in config file, init a child (or children if more than one
    // instance of the process)
	for(int i = 0; i < procCount; i++){
		char message[100];
		strcpy(procToKill, monitorProcs[i].name);
		sleepTime = monitorProcs[i].sleep;

		// get list of PIDs with the process name
		pidList = getPidList(monitorProcs[i].name, &pidCount);

		switch(pidCount){
		// no procceses found with that name
		case 0:
			strcpy(message,"Info: No '");
			strcat(message, monitorProcs[i].name);
			strcat(message, "' processes found.\n");
			timestamp(message);
			*pid = -1;
			break;
		// 1 process found with that name
		case 1:
			pidToKill = pidList[0];
			*childCount = *childCount + 1;
			*pid = fork();

			if(*pid == 0){
				break;
			} else if(*pid > 0){
				childPool[*childCount-1].pid = *pid;
				childPool[*childCount-1].childId = childCount-1;
				childPool[*childCount-1].busy = 1;
			}
			break;
		// more than one process with that name
		// need to realloc childpid array size
		default:
			childPool = realloc(childPool, (arraySize + pidCount)*sizeof(child));
			arraySize = arraySize + pidCount;

			// for each PID associated with the proccess, fork off a child
			for(int j = 0; j < pidCount; j++){
				pidToKill = pidList[j];
				*childCount = *childCount + 1;
				*pid = fork();

				if(*pid == 0){
					break;
				} else if(*pid > 0){
					childPool[*childCount-1].pid = *pid;
					childPool[*childCount-1].childId = childCount-1;
					childPool[*childCount-1].busy = 1;
				}
			}
		}

		free(pidList);

		if(*pid == 0){
			break;
		}
	}
}

// returns a list of PIDs associate with 'procName' and sets arraySize
// to the count of these PIDs
int* getPidList(char* procName,int *arraySize){
	int currentSize = 10;
	int lineCount = 0;

	int *pidList = malloc(currentSize * sizeof(int));
	FILE *fp;
	char line[MAX_PROC_NAME + 30];
	char command[MAX_USER_NAME + 25] = "ps -u ";

	strcat(command, getenv("USER"));
	strcat(command, " | grep -w ");
	strcat(command, procName);

	// Open the command for reading.
	fp = popen(command, "r");
	if (fp == NULL) {
		fprintf(stderr, "Failed to run command\n");
		exit(0);
	}

	// Read the output a line at a time, save the pid's of the matching processes
	while (fgets(line, MAX_PROC_NAME + 30, fp)) {
		lineCount++;
		// realloc if more space needed
		if (lineCount > currentSize){
			currentSize = currentSize*2;
			pidList = realloc(pidList, currentSize * sizeof(int));
		}
		sscanf(line, "%d", &pidList[lineCount-1]);
	}

	// close
	pclose(fp);

	*arraySize = lineCount;

	return pidList;
}

//take string input, timestampt it and print to logfile and stdout
void timestamp(char* input){
	FILE* fp = popen("date", "r");
	if (fp == NULL) {
		fprintf(stderr, "Failed to run command\n");
		exit(0);
	}

	char line[30];
	fgets(line, 128, fp);
	pclose(fp);

	char output[1000] = "[";
	strcat(output, line);
	output[29] = '\0';
	strcat(output, "] ");
	strcat(output, input);
	printf(output);
	fprintf(logfile, output);
	fflush(logfile);
}

// parent waits for children and prints closing remarks to logfile
void parentFinish(pid_t* childpid, int childCount){
	int *status = malloc(childCount * sizeof(int));

	/* wait for all children, save their status to status[i]*/
	for(int i = 0; i< childCount; i++){
		waitpid(childpid[i],&status[i],0);
	}

    /* count how many children returned 1 (killed a process) */
    int killCount = 0;

    for(int i = 0; i < childCount; i++){
    	if (WEXITSTATUS(status[i]) == 1){
    		killCount++;
    	}
    }

    /* print final message */
    char kc[10];
    char message[100] = "Info: Exiting. ";
    sprintf(kc,"%d",killCount);
	strcat(message, kc);
	strcat(message, " process(es) killed.\n");
	timestamp(message);

    cleanup(childpid, status);
}

// cleans up allocated memory and file pointers
void cleanup(int *childpid, int *status){
	/*for(int i = 0; i < procCount; i++){
		//printf("freeing processNames[%d]\n", i);
		free(processNames[i]);
	}*/

	//free(processNames);
	if (childpid != NULL){
			free(childpid);
		}
	if (status != NULL){
		free(status);
	}
	fclose(logfile);
}
