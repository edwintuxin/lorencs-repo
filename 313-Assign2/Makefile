# name: Mikus Lorence
# ONE Card number: 1227388
# Unix id: lorencs

#CFLAGS = -g -O0

psim: psim.o memwatch.o
	gcc $(CFLAGS) -Wall -Wno-format -std=c99 -o psim psim.o -lm

psim.o: psim.c memwatch.h
	gcc $(CFLAGS) -Wall -Wno-format -DMEMWATCH -DMW_STDIO -std=c99 -c psim.c

memwatch.o: memwatch.c memwatch.h
	gcc $(CFLAGS) -std=c99 -Wall -c -DMEMWATCH -DMW_STDIO memwatch.c 
	
clean:
	-rm -f *.exe psim core
	-rm -f *.o psim core

# these are tests that I ran in order to obtain my graphs:
