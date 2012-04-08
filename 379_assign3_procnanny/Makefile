# Name: 				Mikus Lorence
# Studen ID: 			1227388
# UNIX ID: 				lorencs
# Lecture Section:		B1
# Instructor's Name:	Paul Lu
# Lab Section:			H02
# TA's Name:			Arnamoy Bhattacharyya

CFLAGS = -g -O0

server: server.o  serverFunctions.o pipeMessage.o memwatch.o
	gcc $(CFLAGS) -Wall -std=c99 -o procnanny.server server.o serverFunctions.o pipeMessage.o memwatch.o -lm
	
client: client.o  clientFunctions.o child.o pipeMessage.o memwatch.o
	gcc $(CFLAGS) -Wall -std=c99 -o procnanny.client client.o clientFunctions.o child.o pipeMessage.o memwatch.o -lm

server.o: server.c memwatch.h
	gcc $(CFLAGS) -Wall -std=c99 -c -DMEMWATCH -DMW_STDIO server.c
	
client.o: client.c memwatch.h
	gcc $(CFLAGS) -Wall -std=c99 -c -DMEMWATCH -DMW_STDIO client.c
	
child.o: child.c child.h memwatch.h
	gcc $(CFLAGS) -Wall -std=c99 -c -DMEMWATCH -DMW_STDIO child.c
	
serverFunctions.o: serverFunctions.c serverFunctions.h memwatch.h
	gcc $(CFLAGS) -Wall -std=c99 -c -D_XOPEN_SOURCE -DMEMWATCH -DMW_STDIO serverFunctions.c
	
clientFunctions.o: clientFunctions.c clientFunctions.h memwatch.h
	gcc $(CFLAGS) -Wall -std=c99 -c -D_XOPEN_SOURCE -DMEMWATCH -DMW_STDIO clientFunctions.c
	
pipeMessage.o: pipeMessage.c pipeMessage.h memwatch.h
	gcc $(CFLAGS) -Wall -std=c99 -c -DMEMWATCH -DMW_STDIO pipeMessage.c

memwatch.o: memwatch.c memwatch.h
	gcc $(CFLAGS) -std=c99 -Wall -c -DMEMWATCH -DMW_STDIO memwatch.c 

clean:
	-rm -f *.exe procnanny core
	-rm -f *.o server core
	-rm -f *.o client core
	-rm -f *.o procnanny.server core
	-rm -f *.o procnanny.client core
	-rm -f *.o sharedFunctions core
	-rm -f *.o child core
	-rm -f *.o pipeMessage core
	-rm -f *.o *.log memwatch core
   
tar:
	tar cvf submit.tar main.c mainFunctions.c mainFunctions.h child.c child.h pipeMessage.c pipeMessage.h README report.html Makefile        
