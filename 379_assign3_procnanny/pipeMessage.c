/* IPC Messaging Example
 * Author: Pooria Joulani
 * Date: Feb 18 2011
 *
 * Example adopted from:
 * Stevens, Rago, "Advance Programming in the UNIX Environment, Second Edition",2005, (Figure 15.5)
 * Shows a header/payload approach to message passing between processes using pipes.
 */

#include "pipeMessage.h"
#include "memwatch.h"

#define TEXT_MESSAGE 1

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
void write_message(int fd,struct pipeMessage* msg)
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

	msg=malloc(sizeof(struct pipeMessage));

	size = strlen(message);

	msg->body = malloc(sizeof(char)*(size+1));

	msg->header.size = size;
	strcpy(msg->body,message);

	return msg;
}
