#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>

#define d_printf if(DEBUG) printf

/*	Structure to have the array freelist. freelist keeps track of the free 
	blocks in each of the levels of the buddy allocation system*/

typedef struct {
	int max_level;
	void** freelist;
}freelist_t;


typedef struct _stritem {
    struct _stritem *next;
    int             nbytes;     /* size of data */
    /*uint8_t         level;		 which freelist level we're in */
    size_t			size;
    bool in_use;				/* To denote if the block is currently being used or not */
	void *data;
} item;

/* 	To initialize the chunk of memory. Takes in the max size parameter.
	It should init the free list structure. memset.*/

void buddy_init(int max_level);

/* 	As per the request size, find the chunk of memory (next power of 2) 
	and allocate. Update the free lists accordingly.*/
void* buddy_alloc(size_t size);

/* 	As per the request size, allocate the exact amount of memory. Return 
	the extra memory to the corresponding free list levels. */
void* buddy_exact_alloc(void** ptr, size_t size);

/* 	Frees the block of memory. Update the free lists accordingly.*/
void buddy_free(void **ptr);

/* 	Invoked by buddy_free or buddy_alloc. Combines blocks of memory 
to a bigger chunk.*/
void* buddy_merge(size_t size_requested);

/* prints the memory layout at any given point of time */
void print_the_memory_layout();

/* returns the next power of two for a given size */
int get_next_power_of_2(int size);

/* takes size and returns if its a power of two or not. returns 1 if its a power of two and zero otherwise */
int is_power_of_2(int size);