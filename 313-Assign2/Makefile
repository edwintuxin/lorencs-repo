# name: Mikus Lorence
# ONE Card number: 1227388
# Unix id: lorencs

#CFLAGS = -g -O0

psim: psim.o 
	gcc $(CFLAGS) -Wall -Wno-format -std=c99 -o psim psim.o -lm

psim.o: psim.c
	gcc $(CFLAGS) -Wall -Wno-format -std=c99 -c psim.c
	
clean:
	-rm -f *.exe psim core
	-rm -f *.o psim core
	
test:
	./psim T 5 0.04 50,000 1 1 2 3 4 5

# these are tests that I ran in order to obtain my graphs:
