/*
 * pipeMessage.h
 *
 *  Created on: Mar 6, 2012
 *      Author: Mik
 */


#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <signal.h>
#include <unistd.h>
#include <time.h>
#include <sys/time.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/select.h>

#ifndef PIPEMESSAGE_H_
#define PIPEMESSAGE_H_

#endif /* PIPEMESSAGE_H_ */

/* The heeader, including message size and type*/
struct my_header
{
	int size;
};

/* The message, including a header and a body (payload)*/
struct pipeMessage
{
	struct my_header header;
	char* body;
};

struct pipeMessage* read_message(int fd);
void write_message(int fd,struct pipeMessage* msg);
struct pipeMessage* init_message(char* message);
