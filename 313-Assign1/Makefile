# name: Mikus Lorence
# ONE Card number: 1227388
# Unix id: lorencs

#CFLAGS = -g -O0

esim: esim.o 
	gcc $(CFLAGS) -Wall -Wno-format -std=c99 -o esim esim.o 

esim.o: esim.c 
	gcc $(CFLAGS) -Wall -Wno-format -std=c99 -c esim.c
	
clean:
	-rm -f *.exe esim core
	-rm -f *.o esim core
	
run1: 
	./esim 100 1 8000 0.01 100000 5 11 22 33 44 55
	
#tar:
#	tar cvf submit.tar main.c memwatch.c memwatch.h README Makefile
   
#tar:
#	tar cvf submit.tar main.c mainFunctions.c mainFunctions.h memwatch.c memwatch.h Makefile     