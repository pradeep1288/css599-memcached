
CC=gcc
CFLAGS=-Wall -O3


all: buddy-test

buddy-test: test.o buddy.o
	$(CC) $(CFLAGS) -o $@ $? -lm

clean:	
	/bin/rm -f *.o a.out test buddy.o
