
CC=gcc
CFLAGS=-Wall -O3


all: test

buddy-test: test.o buddy-system.o
	$(CC) $(CFLAGS) -o $@ $? -lm

clean:	
	/bin/rm -f *.o a.out test buddy.o
