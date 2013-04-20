CC=gcc
CFLAGS=-Wall -O3 -c -g -O0 -fno-inline

all: buddy-test

buddy-test: buddy.o test.o
	$(CC) buddy.o test.o -o buddy-test -lm

buddy.o: buddy.c
	$(CC) $(CFLAGS) buddy.c 

test.o: test.c
	$(CC) $(CFLAGS) test.c

clean:	
	/bin/rm -Rf *.o a.out* buddy-test