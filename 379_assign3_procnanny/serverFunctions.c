/*
 * serverFunctions.c
 *
 *  Created on: Apr 7, 2012
 *      Author: Mik
 */

#include "serverFunctions.h"
#include "memwatch.h"

extern int procCount;
extern FILE* logfile;
extern FILE *serverlog;
extern char configPath[128];
extern procs monitorProcs[128];
extern int killCount;
extern struct sigaction* newAction[2];
extern int clientCount;
extern int sock;
extern int clients[32];
extern int serverPort;

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
		char message[256];
		char name[MAX_PROC_NAME];
		int sleeptime;

		sscanf(line, "%s %d", name, &sleeptime);

		// warn user that monitoring 'procnanny' will cause unexpected behavior and exit
		if (!strcmp(name, "procnanny")){
			strcpy(message, "Warning: You are attempting to monitor the "
					"'procnanny' process. Exiting to avoid unexpected behavior.\n");
			printToFile(message, 1, 1, logfile);

			procCount = lineCount;
			cleanup();
			exit(0);
		}

		// check if the process name has been already scanned in, ignore it if it has
		if (exists(name, lineCount)){
			strcpy(message, "Warning: Ignoring duplicate process '");
			strcat(message, name);
			strcat(message, "' in the .config file.\n");
			printToFile(message, 0, 1, logfile);
			continue;
		}

		lineCount++;

		// store the process name and sleep time (in network byte order)
		strcpy(monitorProcs[lineCount-1].name, name);
		monitorProcs[lineCount-1].sleep = htonl(sleeptime);
	}

	//set process count
	procCount = lineCount;

	fclose(configFile);
}

// kills all previous instances of procnanny
void killPrevious(char* procname, int parentID){
	int lineCount = 0;
	int killcount = 0;

	// get a (dynamically alloc'd) list of PIDs with the name specified in procname
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
	strcat(output, " previous '");
	strcat(output, procname);
	strcat(output, "' process(es).\n");
	printToFile(output, 0, 1, logfile);

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
		cleanup();
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

// output the pid, node, and port to stdut and to server log file
void outputStartMsg(){
	char message[128] = "procnanny server: PID ";
	int ipid = getpid();
	char cpid[16];
	sprintf(cpid, "%d", ipid);
	strcat(message, cpid);
	strcat(message, " on node ");
	FILE* fp = popen("hostname", "r");
	if (fp == NULL) {
		fprintf(stderr, "Failed to run command\n");
		exit(0);
	}
	char line[32];
	fgets(line, 128, fp);
	pclose(fp);
	line[strlen(line)-1] = '\0';

	strcat(message, line);
	strcat(message, ", port ");
	char charport[16];
	sprintf(charport, "%d", serverPort);
	strcat(message, charport);
	strcat(message, "\n");

	// print timestamped message to stdout
	printToFile(message, 1, 1, NULL);

	strcpy(message, "NODE ");
	strcat(message, line);
	strcat(message, " PID ");
	strcat(message, cpid);
	strcat(message, " PORT ");
	strcat(message, charport);
	strcat(message, "\n");

	printToFile(message, 0, 0, serverlog);
}

// in a loop, listen for incoming connections, handle client messages
void serverLoop(){
	// setup select, etc
	int maxdesc, ret;
	fd_set read_from;
	struct timeval tv;
	maxdesc = getdtablesize();
	tv.tv_sec = 0;
	tv.tv_usec = 0;

	socklen_t fromlength;
	struct	sockaddr_in	master, from;

	// setup listening socket
	sock = socket (AF_INET, SOCK_STREAM, 0);
	if (sock < 0) {
		perror ("Server: cannot open master socket");
		cleanup();
		exit (1);
	}

	master.sin_family = AF_INET;
	master.sin_addr.s_addr = INADDR_ANY;
	master.sin_port = htons (serverPort);

	// attempt to bind it until a free address is found
	while ((bind (sock, (struct sockaddr*) &master, sizeof (master))) == -1) {
		serverPort++;
		master.sin_port = htons (serverPort);
	}

	// print server startup message (once the port is known)
	outputStartMsg();
	// setup socket for listening
	listen(sock, 32);

	// constantly check for new connections on the socket
	// as well as messages from clients
	while (1) {
		FD_ZERO(&read_from);
		FD_SET(sock, &read_from);

		// check if client is attempting to connect
		ret = select(maxdesc, &read_from, NULL, NULL, &tv);

		// accept connection
		if(ret){
			fromlength = sizeof (from);
			clients[clientCount] = accept (sock, (struct sockaddr*) & from, & fromlength);

			if (clients[clientCount] < 0) {
				perror ("Server: accept failed");
				cleanup();
				exit (1);
			}

			sendConfig(clients[clientCount]);

			clientCount++;
			//listen(sock, 32);
		}

		readClientMessages();
	}
}

// read and handle the messages from the clients
void readClientMessages(){
	// setup select
	int maxdesc, ret;
	fd_set read_from;
	struct timeval tv;

	maxdesc = getdtablesize();

	tv.tv_sec = 0;
	tv.tv_usec = 0;

	// check each client for a header message
	for (int i = 0; i < clientCount; i++){
		FD_ZERO(&read_from);
		FD_SET(clients[i], &read_from);

		// check if child 'i' has sent a message
		ret = select(maxdesc, &read_from, NULL, NULL, &tv);

		// read message
		if(ret){
			char header[8];
			// if read returns 0 (client may have been killed), continue to next client
			if(!read (clients[i], header, sizeof(header))){
				continue;
			}

			//if header is "output", receive the next string and print it to logfile
			if (!strcmp(header, "output")){
				char output[256];
				read (clients[i], output, sizeof(output));
				printToFile(output, 0, 0, logfile);
			}
		}
	}
}

// sends the config specs to a client
void sendConfig(int client){
	char header[8];
	strcpy(header, "config");
	write (client, &header, sizeof(header));
	send (client, &monitorProcs, sizeof(monitorProcs), 0);
	write (client, &procCount, sizeof(procCount));
}

// sets the handler '*handler' to handle the 'sigType' signal
void setHandler(int sigType, void* handler, int i){
	newAction[i] = malloc(sizeof(struct sigaction));
	newAction[i]->sa_handler = (handler);
	sigemptyset(&newAction[i]->sa_mask);
	newAction[i]->sa_flags = 0;

	sigaction(sigType, newAction[i], NULL);
}

// handles the SIGINT and SIGHUP signals
void signalHandler(int signalNum){
	if (signalNum == SIGINT){

		// send exit message to all client
		for (int i = 0; i < clientCount; i++){
			char header[8];
			strcpy(header, "exit");
			write(clients[i], header, sizeof(header));
		}

		char nodenames[1024] = "";
		// wait for the clients to respond with their kill counts and
		// record the node names of those clients that had kills
		waitForClients(nodenames);

		// print final message
		char kc[10];
		char message[100] = "Info: Caught SIGINT. Exiting cleanly. ";
		sprintf(kc,"%d",killCount);
		strcat(message, kc);
		if (killCount > 0){
			strcat(message, " process(es) killed on ");
			strcat(message, nodenames);
			strcat(message, ".\n");
		} else {
			strcat(message, " process(es) killed.\n");
		}

		printToFile(message, 1, 1, logfile);

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
		printToFile(message, 1, 1, logfile);

		//send new config file to each client
		for (int i = 0; i < clientCount; i++){
			sendConfig(clients[i]);
		}
	}
}

// wait for the clients to respond with their kill counts and
// record the node names of those clients that had kills
void waitForClients(char* nodenames){
	int maxdesc, ret;
	fd_set read_from;
	struct timeval tv;
	maxdesc = getdtablesize();
	tv.tv_sec = 0;
	tv.tv_usec = 0;

	// array of client ids that have exited
	int exitedClients[32] = {[0 ... 31] = -1};
	int exited = 0; 	/* count of exited clients */

	// while not all clients have exited
	while (exited < clientCount){
		for (int i = 0; i < clientCount; i++){
			// if the client's id is recorded in the array of ids, continue to next client
			if (clientExited(i, exitedClients)){
				continue;
			}

			FD_ZERO(&read_from);
			FD_SET(clients[i], &read_from);

			// check if client send message
			ret = select(maxdesc, &read_from, NULL, NULL, &tv);

			// read message
			if(ret){
				char header[8];

				// read message
				read(clients[i], header, sizeof(header));


				//if header is "output", receive the next string and print it to logfile
				if (!strcmp(header, "output")){
					char output[256];
					read (clients[i], output, sizeof(output));
					printToFile(output, 0, 0, logfile);

				// if header is "exit", confirmation that client has exited
				}else if (!strcmp(header, "exit")){
					int clientKillCount;

					//receive the client's kill count
					read(clients[i], &clientKillCount, sizeof(clientKillCount));
					clientKillCount = ntohl(clientKillCount);

					// if this client has killed procs, append its nodename
					if (clientKillCount > 0){
						killCount = killCount + clientKillCount;
						char nodename[16];
						read(clients[i], nodename, sizeof(nodename));

						//if it is not the first nodename appended, append a comma
						if (strlen(nodenames) > 0){
							strcat(nodenames, ", ");
						}

						strcat(nodenames, nodename);
					}

					exited++;
					exitedClients[exited] = i;
				}
			}
		}
	}
}

// checks if 'num' is in the array 'exitedClients'
int clientExited(int num, int exitedClients[32]){
	int returnVal = 0;

	for (int i = 0; i < 32; i++){
		if (exitedClients[i] == num){
			return 1;
		}
	}

	return returnVal;
}

// take string input, and print to logfile
// if p2stdout is 1, also prints to stdout
// if timestamp is 1, it timestamps it
// if logfile is NULL, don't print it to any file
void printToFile(char* input, int p2stdout, int timestamp, FILE* file){
	char output[1000];

	if (timestamp){
		FILE* fp = popen("date", "r");
		if (fp == NULL) {
			fprintf(stderr, "Failed to run command 'date'\n");
			cleanup();
			exit(0);
		}

		char line[30];
		fgets(line, 128, fp);
		pclose(fp);

		strcpy(output, "[");
		strcat(output, line);
		output[29] = '\0';
		strcat(output, "] ");
		strcat(output, input);
	} else {
		strcpy(output, input);
	}

	if (p2stdout){
		printf(output);
	}
	if (file != NULL){
		fprintf(file, output);
		fflush(file);
	}
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

// cleans up allocated memory, file pointers, and pipes
void cleanup(){
	free(newAction[0]);
	free(newAction[1]);

	fclose(logfile);
	fclose(serverlog);
}
