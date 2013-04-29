CC=gcc
CFLAGS=-Wall -O3 -c -g -O0 -fno-inline

all: powers_of_two_test random_mix_test merge_test

powers_of_two_test: buddy.o powers_of_two_test.o
	$(CC) buddy.o powers_of_two_test.o -o powers_of_two_test -lm

random_mix_test: buddy.o random_mix_test.o
	$(CC) buddy.o random_mix_test.o -o random_mix_test -lm

merge_test: buddy.o merge_test.o
	$(CC) buddy.o merge_test.o -o merge_test -lm

buddy.o: buddy.c
	$(CC) $(CFLAGS) buddy.c 

powers_of_two_test.o: powers_of_two_test.c
	$(CC) $(CFLAGS) powers_of_two_test.c

random_mix_test.o: random_mix_test.c
	$(CC) $(CFLAGS) random_mix_test.c

merge_test.o: merge_test.c
	$(CC) $(CFLAGS) merge_test.c

clean:	
	/bin/rm -Rf *.o a.out* buddy-test