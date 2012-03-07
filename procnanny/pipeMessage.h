/*
 * pipeMessage.h
 *
 *  Created on: Mar 6, 2012
 *      Author: Mik
 */

#ifndef PIPEMESSAGE_H_
#define PIPEMESSAGE_H_



#endif /* PIPEMESSAGE_H_ */

struct my_header
{
	int size;
	enum message_type type;
};

/* The message, including a header and a body (payload)*/
struct pipeMessage
{
	struct my_header header;
	char* body;
};

struct pipeMessage* read_message(int fd);
int write_message(int fd,struct pipeMessage* msg);
struct pipeMessage* init_message(char* message);
