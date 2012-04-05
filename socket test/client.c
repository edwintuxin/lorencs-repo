/*
 * client.c
 *
 *  Created on: Apr 5, 2012
 *      Author: lorencs
 */
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <stdio.h>

int myport;

int main(int argc, char* argv[]) {
	if (argc < 2){
		printf("no port\n");
		exit(0);
	}

	sscanf(argv[1], "%d", &myport);

	int	s;

	struct	sockaddr_in	server;

	struct	hostent		*host;

	/* Put here the name of the sun on which the server is executed */
	host = gethostbyname ("ui17");

	if (host == NULL) {
		perror ("Client: cannot get host description");
		exit (1);
	}

	s = socket (AF_INET, SOCK_STREAM, 0);

	if (s < 0) {
		perror ("Client: cannot open socket");
		exit (1);
	}

	bzero (&server, sizeof (server));
	bcopy (host->h_addr, & (server.sin_addr), host->h_length);
	server.sin_family = host->h_addrtype;
	server.sin_port = htons (myport);

	if (connect (s, (struct sockaddr*) & server, sizeof (server))) {
		perror ("Client: cannot connect to server");
		exit (1);
	}

	while (1) {
		char message[100];
		read (s, message, sizeof (message));

		printf(message);

	}

	return 0;
}
