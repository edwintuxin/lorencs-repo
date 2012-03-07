/* IPC Messaging Example
 * Author: Pooria Joulani
 * Date: Feb 18 2011
 *
 * Example adopted from:
 * Stevens, Rago, "Advance Programming in the UNIX Environment, Second Edition",2005, (Figure 15.5)
 * Shows a header/payload approach to message passing between processes using pipes.
 */

#include <stdio.h>
#include "pipeMessage.h"
#include "memwatch.h"

#define TEXT_MESSAGE 1

/*The type of the message bein sent*/
enum message_type {textMsg, dataMsg};

/* The heeader, including message size and type*/
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

/* Reads a message from the file descriptor fd*/
struct pipeMessage* read_message(int fd)
{
	int n;
	struct pipeMessage* msg;

	msg = malloc(sizeof(struct pipeMessage));
	/*Read the header*/
	n=read(fd,&msg->header,sizeof(msg->header));
	if(n!=sizeof(msg->header))
		fprintf(stderr, "bad message format");

	/*Allocate memory for and read the body*/
	msg->body = malloc((msg->header.size+1)*sizeof(char));
	memset(msg->body,0,msg->header.size+1);
	n=read(fd,msg->body,msg->header.size);
	if(n!=msg->header.size)
		fprintf(stderr, "incomplete message");

	return msg;
};

/* Writes the message in msg to file descriptor fd*/
int write_message(int fd,struct pipeMessage* msg)
{
	/* Write header*/
	write(fd,&msg->header,sizeof(msg->header));
	/*Write payload*/
	write(fd,msg->body,msg->header.size);
};

/*Creates a text message containing the text given in the argument*/
struct pipeMessage* init_message(char* message)
{
	struct pipeMessage* msg;
	int size;
	enum message_type type=textMsg;/* Creating a text message*/


	msg=malloc(sizeof(struct pipeMessage));

	size = strlen(message);

	msg->body = malloc(sizeof(char)*(size+1));

	msg->header.size = size;
	msg->header.type = type;
	strcpy(msg->body,message);

	return msg;
}
