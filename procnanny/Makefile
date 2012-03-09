# Name: 				Mikus Lorence
# Studen ID: 			1227388
# UNIX ID: 				lorencs
# Lecture Section:		B1
# Instructor's Name:	Paul Lu
# Lab Section:			H02
# TA's Name:			Arnamoy Bhattacharyya

#CFLAGS = -g -O0

procnanny: main.o  mainFunctions.o child.o pipeMessage.o memwatch.o
	gcc $(CFLAGS) -Wall -std=c99 -o procnanny main.o mainFunctions.o child.o pipeMessage.o memwatch.o -lm

main.o: main.c memwatch.h
	gcc $(CFLAGS) -Wall -std=c99 -c -DMEMWATCH -DMW_STDIO main.c
	
child.o: child.c child.h memwatch.h
	gcc $(CFLAGS) -Wall -std=c99 -c -DMEMWATCH -DMW_STDIO child.c
	
mainFunctions.o: mainFunctions.c mainFunctions.h memwatch.h
	gcc $(CFLAGS) -Wall -std=c99 -c -D_XOPEN_SOURCE -DMEMWATCH -DMW_STDIO mainFunctions.c
	
pipeMessage.o: pipeMessage.c pipeMessage.h memwatch.h
	gcc $(CFLAGS) -Wall -std=c99 -c -DMEMWATCH -DMW_STDIO pipeMessage.c

memwatch.o: memwatch.c memwatch.h
	gcc $(CFLAGS) -std=c99 -Wall -c -DMEMWATCH -DMW_STDIO memwatch.c 

clean:
	-rm -f *.exe procnanny core
	-rm -f *.o main core
	-rm -f *.o mainFunctions core
	-rm -f *.o child core
	-rm -f *.o pipeMessage core
	-rm -f *.o *.log memwatch core
   
tar:
	tar cvf submit.tar main.c mainFunctions.c mainFunctions.h child.c child.h pipeMessage.c pipeMessage.h memwatch.c memwatch.h README report.html Makefile        
