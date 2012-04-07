/*
 * server.c
 *
 *  Created on: Apr 5, 2012
 *      Author: lorencs
 */

#include <stdlib.h>
#include <string.h>
#include <sys/select.h>
#include <termios.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <stdio.h>
#include <signal.h>
#include <setjmp.h>
#include <unistd.h>
#include <time.h>
#include <sys/time.h>
#include <sys/wait.h>

int myport;
int clients[32];
int clientCount;
int sock;
struct sigaction* newAction;	/* action struct to use with signal handler */

void setHandler(int sigType, void* handler) {
	newAction = malloc(sizeof(struct sigaction));
	newAction->sa_handler = (handler);
	sigemptyset(&newAction->sa_mask);
	newAction->sa_flags = 0;

	sigaction(sigType, newAction, NULL);
}

void signalHandler(int signalNum) {
	if (signalNum == SIGINT){
		close(sock);
	}
}

int main(int argc, char *argv[]){
	if (argc < 2){
		printf("no port\n");
		exit(0);
	}

	int maxdesc, ret;
	fd_set read_from;
	struct timeval tv;
	maxdesc = getdtablesize();
	tv.tv_sec = 0;
	tv.tv_usec = 0;

	setHandler(SIGINT, signalHandler);

	clientCount = 0;
	sscanf(argv[1], "%d", &myport);

	int	fromlength, number, outnum;

	struct	sockaddr_in	master, from;

	sock = socket (AF_INET, SOCK_STREAM, 0);

	if (sock < 0) {
		perror ("Server: cannot open master socket");
		exit (1);
	}

	master.sin_family = AF_INET;
	master.sin_addr.s_addr = INADDR_ANY;
	master.sin_port = htons (myport);

	while ((bind (sock, (struct sockaddr*) &master, sizeof (master))) == -1) {
		char msg[64];
		strcpy(msg, "Server: cannot bind master socket on port ");
		char port[4];
		sprintf(port, "%d", myport);
		strcat(msg, port);
		perror (msg);
		myport++;
		master.sin_port = htons (myport);
	}

	listen(sock, 1);

	while (1) {
		FD_ZERO(&read_from);
		FD_SET(sock, &read_from);

		// check if child send message
		ret = select(maxdesc, &read_from, NULL, NULL, &tv);

		// read message
		if(ret){
			fromlength = sizeof (from);
			clients[clientCount] = accept (sock, (struct sockaddr*) & from, & fromlength);

			if (clients[clientCount] < 0) {
				perror ("Server: accept failed");
				exit (1);
			}

			clientCount++;

			printf("client %d connected, sending msg to all clients\n", clientCount);

			for (int i = 0; i < clientCount; i++){
				write (clients[i], "hello sir\n", sizeof ("hello sir\n"));
			}

			listen(sock, 1);
		}
	}

    return 0;
}
