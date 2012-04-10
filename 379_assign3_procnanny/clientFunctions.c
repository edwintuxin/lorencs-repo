/*
 * clientFunctions.c
 *
 *  Created on: Apr 7, 2012
 *      Author: Mik
 */

#include "clientFunctions.h"
#include "memwatch.h"

extern int sleepTime;
extern int procCount;
extern procs monitorProcs[128];
extern int childCount;
extern int idleChildCount;
extern child *childPool;
extern struct pipeMessage* msg;
extern int killCount;
extern char hostname[128];
extern int serverPort;
extern int sock;

void connectToServer(){
	struct	sockaddr_in	server;
	struct	hostent		*host;

	host = gethostbyname(hostname);
	if (host == NULL) {
		perror ("Client: cannot get host description");
		exit (1);
	}

	// establish socket
	sock = socket (AF_INET, SOCK_STREAM, 0);

	if (sock < 0) {
		perror ("Client: cannot open socket");
		exit (1);
	}

	bzero(&server, sizeof (server));
	bcopy(host->h_addr, & (server.sin_addr), host->h_length);
	server.sin_family = host->h_addrtype;
	server.sin_port = htons (serverPort);

	if (connect (sock, (struct sockaddr*) & server, sizeof (server))) {
		perror ("Client: cannot connect to server");
		exit (1);
	}
}

void receiveConfig(){
	int maxdesc, ret;
	fd_set read_from;
	struct timeval tv;
	maxdesc = getdtablesize();
	tv.tv_sec = 0;
	tv.tv_usec = 0;

	while (1) {
		char header[8];

		FD_ZERO(&read_from);
		FD_SET(sock, &read_from);

		// check if child send message
		ret = select(maxdesc, &read_from, NULL, NULL, &tv);

		// read message
		if(ret){
			read (sock, header, sizeof(header));

			if (!strcmp(header, "config")){
				recv (sock, &monitorProcs, sizeof(monitorProcs), 0);
				read (sock, &procCount, sizeof(procCount));
				for(int i = 0; i < procCount; i++){
					monitorProcs[i].sleep = ntohl(monitorProcs[i].sleep);
					printf("%s :", monitorProcs[i].name);
					printf("%d\n", monitorProcs[i].sleep);
				}
			}
		break;
		}

	}
}

// kills all previous instances of procnanny
void killPrevious(char* procname, int parentID){
	int lineCount = 0;
	int killcount = 0;

	// get a (dynamically alloc'd) list of PIDs with the proc name 'procnanny'
	// save the count of these PIDs to lineCount
	int *pidList = getPidList(procname, &lineCount);

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
	pidList = getPidList(procname, &lineCount);

	char output[1024] = "";
	char kc[16];

	// if more than 1 'procnanny' running, kill was unsuccessful
	if (lineCount > 1){
		strcat(output, "Info: Unable to clean up ");
		sprintf(kc,"%d", lineCount - 1);
		strcat(output, kc);
	// else print successful clean message
	} else {
		strcat(output, "Action: Cleaned up ");
		sprintf(kc,"%d",killcount);
		strcat(output, kc);
	}
	strcat(output, " previous '");
	strcat(output, procname);
	strcat(output, "' process(es) on node ");
	char hostname[32];
	getHostName(hostname);
	strcat(output, hostname);
	strcat(output, ".\n");
	timestampToServer(output);

	free(pidList);
}

// returns a list of PIDs associate with 'procName' and sets arraySize
// to the count of these PIDs
int* getPidList(char* procName,int *arraySize){
	int currentSize = 10;
	int lineCount = 0;

	int *pidList = malloc(currentSize * sizeof(int));
	FILE *fp;
	char line[128];
	char command[256] = "ps n -o pid,command -u ";

	// execute "ps" and "grep" commands
	strcat(command, getenv("USER"));
	strcat(command, " | grep -w ");
	strcat(command, procName);
	strcat(command, " | grep -v 'grep'");

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

// initializes all the children, returns the array of child PIDs
// take pointer to pid (main needs the modified value of it) and pointer to childCount
void initChildren(int *pid, int *_c2p, int *_p2c){
	char procToKill[MAX_PROC_NAME];
    int *pidList;
    int pidCount;
    int arraySize = procCount;
    char pidString[128];

    //printf("procCount is %d\n", procCount);
    // dynamic array to hold the pids of children
	childPool = malloc(procCount*sizeof(child));

    // for each process in config file, init a child (or children if more than one
    // instance of the process)
	for(int i = 0; i < procCount; i++){
		char message[128];

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
			timestampToServer(message);
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

				char writeStr[128];
				strcpy(writeStr, "monitor ");
				strcat(writeStr, monitorProcs[i].name);
				strcat(writeStr, " ");
				strcat(writeStr, pidString);
				strcat(writeStr, " ");
				char sleepStr[16];
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

					char writeStr[128];
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

// parent rechecks for new processes to monitor every 5 secs
void parentLoop(){
	while(1){
		sleep(5);				/* sleep 5 seconds */
		readServerMessages();
		readChildMessages();	/* check if any children have left messages and process them */
		rescanProcs();			/* look for the processes in the most recent config file */
	}
}

void readServerMessages(){
	int maxdesc, ret;
	fd_set read_from;
	struct timeval tv;
	maxdesc = getdtablesize();
	tv.tv_sec = 0;
	tv.tv_usec = 0;

	char header[8];

		FD_ZERO(&read_from);
		FD_SET(sock, &read_from);

		// check if child send message
		ret = select(maxdesc, &read_from, NULL, NULL, &tv);

		// read message
		if(ret){
			read (sock, header, sizeof(header));

			if (!strcmp(header, "config")){
				recv (sock, &monitorProcs, sizeof(monitorProcs), 0);
				read (sock, &procCount, sizeof(procCount));
				for(int i = 0; i < procCount; i++){
					monitorProcs[i].sleep = ntohl(monitorProcs[i].sleep);
					printf("%s :", monitorProcs[i].name);
					printf("%d\n", monitorProcs[i].sleep);
				}
			} else if (!strcmp(header, "exit")){
				cleanExit();
			}
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

			// increment idle child count and set the child's m_pid to 0
			// if message was "available 1", also increment kill count
			if (!strcmp(message, "available 1")){
				idleChildCount++;
				childPool[i].m_pid = 0;
				killCount++;
			} else if (!strcmp(message, "available 0")){
				idleChildCount++;
				childPool[i].m_pid = 0;
			} else if (!strcmp(message, "output")){
				// receive the output string from chlid
				char output[256];
				memset(output, 0, 256);
				msg = read_message(childPool[i].c2p[0]);
				strcpy(output, msg->body);
				resetMsg();

				// send it to server
				char header[8];
				strcpy(header, "output");
				write (sock, header, sizeof(header));
				write (sock, output, sizeof(output));
			}
		}

	}
}

// similar to initializeChildren()
// assigns existing or new children to the approriate processes to monitor
void rescanProcs(){
	// for each process in config file
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
			//print "no process found" message
			strcpy(message,"Info: No '");
			strcat(message, monitorProcs[i].name);
			strcat(message, "' processes found on node ");
			char hostname[32];
			getHostName(hostname);
			strcat(message, hostname);
			strcat(message, ".\n");
			timestampToServer(message);
			pid = -1;
			break;
		// 1 process found with that name
		case 1:
			// if process is being monitored, break
			if (processMonitored(pidList[0])){
				break;
			}

			sprintf(pidString, "%d", pidList[0]);

			// setup the message to send to child
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

				// send message to idle child
				msg = init_message(writeStr);
				write_message(childPool[idleIndex].p2c[1],msg);
				resetMsg();
			} else {
				// allocate space for a new child in the struct
				childCount++;
				childPool = realloc(childPool, childCount*sizeof(child));

				// setup pipes
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

	// -1 if no idle child found
	return -1;
}

//returns 1 if the process pid is already being monitored; 0 otherwise
int processMonitored(int pid){
	for (int i = 0; i < childCount; i++){
		if (childPool[i].m_pid == pid){
			return 1;
		}
	}

	return 0;
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
	// while not all children have "exit completed"
	while (exited < childCount){
		for (int i = 0; i < childCount; i++){
			FD_ZERO(&read_from);
			FD_SET(childPool[i].c2p[0], &read_from);

			// check if child send message
			ret = select(maxdesc, &read_from, NULL, NULL, &tv);

			// read message
			if(ret){
				char message[256];
				memset(message, 0, 256);

				// read message
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
				// if child has exited, increment the exited count
				} else if (!strcmp(message, "exit complete")){
					exited++;
				}
			}
		}
	}
}

// timestamps a message and passes it to server
void timestampToServer(char* input){
	FILE* fp = popen("date", "r");
	if (fp == NULL) {
		fprintf(stderr, "Failed to run command 'date'\n");
		exit(0);
	}

	char line[30];
	fgets(line, 128, fp);
	pclose(fp);

	char output[256] = "[";
	strcat(output, line);
	output[29] = '\0';
	strcat(output, "] ");
	strcat(output, input);

	// send an "output" header
	// and the output string
	char header[8];
	strcpy(header, "output");
	write (sock, header, sizeof(header));
	write (sock, output, sizeof(output));
}

// copies the hostname to the specified string
void getHostName(char *name){
	FILE* fp = popen("hostname", "r");
	if (fp == NULL) {
		fprintf(stderr, "Failed to run command\n");
		exit(0);
	}
	char line[32];
	fgets(line, 128, fp);
	pclose(fp);
	line[strlen(line)-1] = '\0';

	strcpy(name, line);
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

// free memory allocated for the message and set it to NULL
// so that the msg pointer can be reused many times by a process
void resetMsg(){
	free(msg->body);
	free(msg);
	msg = NULL;
}

void cleanExit(){

	// send exit message to all children
	for (int i = 0; i < childCount; i++){
		msg = init_message("exit");
		write_message(childPool[i].p2c[1],msg);
		resetMsg();
	}

	// wait for the children to send their kill messages ("available")
	// and their "exit complete" messages
	waitForChildren();

	// send "exit complete" message to server
	char header[8];
	strcpy(header, "exit");
	write(sock, header, sizeof(header));
	int killCountNet = htonl(killCount);
	write(sock, &killCountNet, sizeof(killCountNet));

	// if this node has killed procs, then also send the hostname of this node
	if (killCount > 0){
		char nodename[16];
		getHostName(nodename);
		write(sock, nodename, sizeof(nodename));
	}

	shutdown(sock, 2);
	cleanup();
	exit(1);
}

// cleans up allocated memory, file pointers, and pipes
void cleanup(){
	if (childPool != NULL){
		free(childPool);
	}

	if (msg != NULL){
		free(msg->body);
		free(msg);
	}

	for (int i = 0; i < childCount; i++){
		close(childPool[i].p2c[0]);
		close(childPool[i].p2c[1]);
		close(childPool[i].c2p[0]);
		close(childPool[i].c2p[1]);
	}
}
