CC=gcc
CFLAGS=-Wall -O3 -c -g -O0 -fno-inline

all: buddy-alloc

buddy-alloc: buddy.o
	$(CC) buddy.o -o buddy-alloc

buddy.o: buddy.c
	$(CC) $(CFLAGS) buddy.c 

clean:	
	/bin/rm -Rf *.o a.out* buddy-alloc