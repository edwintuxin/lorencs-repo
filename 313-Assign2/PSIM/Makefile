# name: Mikus Lorence
# ONE Card number: 1227388
# Unix id: lorencs

#CFLAGS = -g

psim: psim.o frame.o memwatch.o
	gcc $(CFLAGS) -Wall -Wno-format -std=c99 -o psim psim.o frame.o memwatch.o -lm

psim.o: psim.c memwatch.h
	gcc $(CFLAGS) -Wall -Wno-format -c -DMEMWATCH -DMW_STDIO -std=c99 psim.c
	
frame.o: frame.c memwatch.h
	gcc $(CFLAGS) -Wall -Wno-format -c -DMEMWATCH -DMW_STDIO -std=c99 frame.c

memwatch.o: memwatch.c memwatch.h
	gcc $(CFLAGS) -std=c99 -Wall -c -DMEMWATCH -DMW_STDIO memwatch.c 
		
clean:
	-rm -f *.exe psim core
	-rm -f *.o psim core
	-rm -f *.o frame core
	-rm -f *.o test core
	-rm -f *.o *.log memwatch core

runT:
	./psim T 20 0.04 50000 5 1 2 3 4 5
	
runP:
	./psim P 20 0.0195 50000 5 1 2 3 4 5
	
runI:
	./psim I 20 0.01 50000 5 1 2 3 4 5

runB:
	./psim B 20 0.05 50000 1 1 2 3 4 5
	
runTest:
	./psim B 20 0.01 50000 2 1 2 3 4 5
