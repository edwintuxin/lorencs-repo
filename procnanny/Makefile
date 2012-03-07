# name: Mikus Lorence
# ONE Card number: 1227388
# Unix id: lorencs

CFLAGS = -g -O0

procnanny: main.o  mainFunctions.o child.o pipeMessage.o memwatch.o
	gcc $(CFLAGS) -Wall -std=c99 -o procnanny main.o mainFunctions.o child.o pipeMessage.o memwatch.o -lm

main.o: main.c memwatch.h
	gcc $(CFLAGS) -Wall -std=c99 -c -DMEMWATCH -DMW_STDIO main.c
	
child.o: child.c child.h memwatch.h
	gcc $(CFLAGS) -Wall -std=c99 -c -DMEMWATCH -DMW_STDIO child.c
	
mainFunctions.o: mainFunctions.c mainFunctions.h memwatch.h
	gcc $(CFLAGS) -Wall -std=c99 -c -DMEMWATCH -DMW_STDIO mainFunctions.c
	
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
	
#tar:
#	tar cvf submit.tar main.c memwatch.c memwatch.h README Makefile
   
tar:
	tar cvf submit.tar main.c mainFunctions.c mainFunctions.h memwatch.c memwatch.h Makefile        
