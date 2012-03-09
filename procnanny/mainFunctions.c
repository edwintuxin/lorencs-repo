#include "mainFunctions.h"
#include "memwatch.h"

//global variables from main
extern int sleepTime;
extern int procCount;
extern FILE* logfile;
extern char configPath[128];
extern procs monitorProcs[128];
extern int childCount;
extern int idleChildCount;
extern child *childPool;
extern struct pipeMessage* msg;
extern int killCount;
extern struct sigaction* newAction[2];

//reads the config file and stores the info from it
void readFile(){
	FILE* configFile = fopen(configPath, "r");

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
			cleanup(NULL);
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
void initChildren(int *pid, int *_c2p, int *_p2c){
	char procToKill[MAX_PROC_NAME];
    int *pidList;
    int pidCount;
    int arraySize = procCount;
    char pidString[100];

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
			sprintf(pidString, "%d", pidList[0]);
			childCount = childCount + 1;
			pipe(childPool[childCount-1].p2c);				/* open pipes for communication */
			pipe(childPool[childCount-1].c2p);
			_c2p[0] = childPool[childCount-1].c2p[0];		/* save the file descriptors to pass to child */
			_c2p[1] = childPool[childCount-1].c2p[1];
			_p2c[0] = childPool[childCount-1].p2c[0];
			_p2c[1] = childPool[childCount-1].p2c[1];

			*pid = fork();

			if(*pid == 0){									/* break if child */
				break;
			} else if(*pid > 0){
				childPool[childCount-1].m_pid = pidList[0];	/* set pid of the process the child will be monitoring */

				char writeStr[100];
				strcpy(writeStr, "monitor ");
				strcat(writeStr, monitorProcs[i].name);
				strcat(writeStr, " ");
				strcat(writeStr, pidString);
				strcat(writeStr, " ");
				char sleepStr[10];
				sprintf(sleepStr, "%d", sleepTime);
				strcat(writeStr, sleepStr);					/* compile "monitor" messages */

				msg = init_message(writeStr);				/* send message */
				write_message(childPool[childCount-1].p2c[1],msg);
				resetMsg();
			}
			break;
		// more than one process with that name
		// need to realloc childPool array size
		default:
			childPool = realloc(childPool, (arraySize + pidCount)*sizeof(child));
			arraySize = arraySize + pidCount;

			// for each PID associated with the proccess, fork off a child
			for(int j = 0; j < pidCount; j++){
				sprintf(pidString, "%d", pidList[j]);
				childCount = childCount + 1;
				pipe(childPool[childCount-1].p2c);
				pipe(childPool[childCount-1].c2p);
				_c2p[0] = childPool[childCount-1].c2p[0];
				_c2p[1] = childPool[childCount-1].c2p[1];
				_p2c[0] = childPool[childCount-1].p2c[0];
				_p2c[1] = childPool[childCount-1].p2c[1];

				*pid = fork();

				if(*pid == 0){
					break;
				} else if(*pid > 0){
					childPool[childCount-1].m_pid = pidList[j];

					char writeStr[100];
					strcpy(writeStr, "monitor ");
					strcat(writeStr, monitorProcs[i].name);
					strcat(writeStr, " ");
					strcat(writeStr, pidString);
					strcat(writeStr, " ");
					char sleepStr[10];
					sprintf(sleepStr, "%d", sleepTime);
					strcat(writeStr, sleepStr);

					msg = init_message(writeStr);
					write_message(childPool[childCount-1].p2c[1],msg);
					resetMsg();
				}
			}
			break;
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

	// execute "ps" and "grep" commands
	strcat(command, getenv("USER"));
	strcat(command, " | grep -w ");
	strcat(command, procName);

	// open the command for reading
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

// parent rechecks for new processes to monitor every 5 secs
void parentLoop(){
	while(1){
		sleep(5);				/* sleep 5 seconds */
		readChildMessages();	/* check if any children have left messages and process them */

		rescanProcs();			/* look for the processes in the most recent config file */
	}
}

// check (once) if any children have left messages and process them
void readChildMessages(){
	// setup select
	int maxdesc, ret;
	fd_set read_from;
	struct timeval tv;

	maxdesc = getdtablesize();

	tv.tv_sec = 0;
	tv.tv_usec = 0;

	for (int i = 0; i < childCount; i++){
		FD_ZERO(&read_from);
		FD_SET(childPool[i].c2p[0], &read_from);

		// check if child 'i' has sent a message
		ret = select(maxdesc, &read_from, NULL, NULL, &tv);

		// read message
		if(ret){
			char message[256];
			memset(message, 0, 256);
			msg = read_message(childPool[i].c2p[0]);
			strcpy(message, msg->body);
			resetMsg();

			//printf("[Parent]: Message is \"%s\"\n", message);

			if (!strcmp(message, "available 1")){
				idleChildCount++;
				childPool[i].m_pid = 0;
				killCount++;
			} else if (!strcmp(message, "available 0")){
				idleChildCount++;
				childPool[i].m_pid = 0;
			}
		}

	}
}

void rescanProcs(){
		// check each process in config file
	for(int i = 0; i < procCount; i++){
		int pid = -1;
		int pidCount;
		char pidString[100];
		char procToKill[MAX_PROC_NAME];
		int _c2p[2];
		int _p2c[2];
		int *pidList;

		char message[100];		/* message to output to logfile */

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
			pid = -1;
			break;
		// 1 process found with that name
		case 1:
			if (processMonitored(pidList[0])){
				break;
			}

			sprintf(pidString, "%d", pidList[0]);

			// setup the message to send
			char writeStr[100];
			strcpy(writeStr, "monitor ");
			strcat(writeStr, monitorProcs[i].name);
			strcat(writeStr, " ");
			strcat(writeStr, pidString);
			strcat(writeStr, " ");
			char sleepStr[10];
			sprintf(sleepStr, "%d", sleepTime);
			strcat(writeStr, sleepStr);

			//reuse idle child, or fork off new one
			if (idleChildCount > 0){
				idleChildCount--;
				int idleIndex = getIdleChildIndex();

				childPool[idleIndex].m_pid = pidList[0];

				msg = init_message(writeStr);
				write_message(childPool[idleIndex].p2c[1],msg);
				resetMsg();
			} else {
				childCount++;
				childPool = realloc(childPool, childCount*sizeof(child));

				pipe(childPool[childCount-1].p2c);
				pipe(childPool[childCount-1].c2p);
				_c2p[0] = childPool[childCount-1].c2p[0];
				_c2p[1] = childPool[childCount-1].c2p[1];
				_p2c[0] = childPool[childCount-1].p2c[0];
				_p2c[1] = childPool[childCount-1].p2c[1];

				pid = fork();

				if (pid == 0){
					break;
				} else if (pid > 0){
					childPool[childCount-1].m_pid = pidList[0];

					msg = init_message(writeStr);
					write_message(childPool[childCount-1].p2c[1],msg);
					resetMsg();
				}
			}
			break;
		// more than one process with that name
		default:
			// for each PID associated with the proccess
			for(int j = 0; j < pidCount; j++){
				//break if it is already monitored
				if (processMonitored(pidList[j])){
					continue;
				}

				// setup the message to send
				sprintf(pidString, "%d", pidList[j]);
				char writeStr[100];
				strcpy(writeStr, "monitor ");
				strcat(writeStr, monitorProcs[i].name);
				strcat(writeStr, " ");
				strcat(writeStr, pidString);
				strcat(writeStr, " ");
				char sleepStr[10];
				sprintf(sleepStr, "%d", sleepTime);
				strcat(writeStr, sleepStr);

				//reuse idle child, or fork off new one
				if (idleChildCount > 0){
					idleChildCount--;
					int idleIndex = getIdleChildIndex();

					childPool[idleIndex].m_pid = pidList[j];

					msg = init_message(writeStr);
					write_message(childPool[idleIndex].p2c[1],msg);
					resetMsg();
				} else {
					childCount++;
					childPool = realloc(childPool, childCount*sizeof(child));

					pipe(childPool[childCount-1].p2c);
					pipe(childPool[childCount-1].c2p);
					_c2p[0] = childPool[childCount-1].c2p[0];
					_c2p[1] = childPool[childCount-1].c2p[1];
					_p2c[0] = childPool[childCount-1].p2c[0];
					_p2c[1] = childPool[childCount-1].p2c[1];

					pid = fork();

					if(pid == 0){
						break;
					} else if(pid > 0){
						childPool[childCount-1].m_pid = pidList[j];

						msg = init_message(writeStr);
						write_message(childPool[childCount-1].p2c[1],msg);
						resetMsg();
					}
				}
			}
			break;
		}

		free(pidList);

		// if child, run the child code
		if(pid == 0){
			childExec(_c2p, _p2c);
		}
	}
}

//returns the index of the first idle child it finds
int getIdleChildIndex(){
	for (int i = 0; i < childCount; i++){
		if (childPool[i].m_pid == 0){
			return i;
		}
	}

	return -1;
}

//returns 1 if the process pid is already being monitored
int processMonitored(int pid){
	for (int i = 0; i < childCount; i++){
		if (childPool[i].m_pid == pid){
			return 1;
		}
	}

	return 0;
}


void signalHandler(int signalNum) {
	if (signalNum == SIGINT){
		// send exit message to children
		for (int i = 0; i < childCount; i++){
			msg = init_message("exit");
			write_message(childPool[i].p2c[1],msg);
			resetMsg();
		}

		waitForChildren();

		/* print final message */
		char kc[10];
		char message[100] = "Info: Caught SIGINT. Exiting cleanly. ";
		sprintf(kc,"%d",killCount);
		strcat(message, kc);
		strcat(message, " process(es) killed.\n");
		timestamp(message);

		cleanup();
		exit(EXIT_SUCCESS);
	}
	if (signalNum == SIGHUP){
		// re-read file, print message
		readFile();
		char message[100];
		strcpy(message, "Info: Caught SIGHUP. Configuration file '");
		strcat(message, configPath);
		strcat(message, "' re-read.\n");
		timestamp(message);
	}
}

// check for all children's "exit complete" messages and at the same
// time listen for "available 1/0" messages
void waitForChildren(){
	int maxdesc, ret;
	fd_set read_from;
	struct timeval tv;
	maxdesc = getdtablesize();
	tv.tv_sec = 0;
	tv.tv_usec = 0;

	int exited = 0; 	/* count of exited children */
	while (exited < childCount){
		for (int i = 0; i < childCount; i++){
			FD_ZERO(&read_from);
			FD_SET(childPool[i].c2p[0], &read_from);

			ret = select(maxdesc, &read_from, NULL, NULL, &tv);

			// read message
			if(ret){
				char message[256];
				memset(message, 0, 256);
				msg = read_message(childPool[i].c2p[0]);
				strcpy(message, msg->body);
				resetMsg();

				// handle "available" message
				if (!strcmp(message, "available 1")){
					idleChildCount++;
					childPool[i].m_pid = 0;
					killCount++;
				} else if (!strcmp(message, "available 0")){
					idleChildCount++;
					childPool[i].m_pid = 0;
				} else if (!strcmp(message, "exit complete")){
					exited++;
				}
			}
		}
	}
}

void setHandler(int sigType, void* handler, int i) {


	newAction[i] = malloc(sizeof(struct sigaction));
	newAction[i]->sa_handler = (handler);
	sigemptyset(&newAction[i]->sa_mask);
	newAction[i]->sa_flags = 0;

	sigaction(sigType, newAction[i], NULL);
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

// take string input, timestampt it and print to logfile and stdout
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
	//if (p2stdout){
		printf(output);
	//}
	fprintf(logfile, output);
	fflush(logfile);
}

// cleans up allocated memory and file pointers
void cleanup(){
	if (childPool != NULL){
		free(childPool);
	}

	if (msg != NULL){
		free(msg->body);
		free(msg);
	}

	free(newAction[0]);
	free(newAction[1]);

	fclose(logfile);

	for (int i = 0; i < childCount; i++){
		close(childPool[i].p2c[0]);
		close(childPool[i].p2c[1]);
		close(childPool[i].c2p[0]);
		close(childPool[i].c2p[1]);
	}
}

void resetMsg(){
	free(msg->body);
	free(msg);
	msg = NULL;
}
