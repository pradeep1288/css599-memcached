#include <stdint.h>
#include <stdlib.h>

/*	Structure to have the array freelist. freelist keeps track of the free 
	blocks in each of the levels of the buddy allocation system*/

typedef struct {
	int max_level;
	void** freelist;
}freelist_t;


typedef struct _stritem {
    struct _stritem *next;
    int             nbytes;     /* size of data */
    uint8_t         level;		/* which freelist level we're in */
	void *data;
} item;

/* 	To initialize the chunk of memory. Takes in the max size parameter.
	It should init the free list structure. memset.*/

void buddy_init();

/* 	As per the request size, find the chunk of memory and allocate. Update 
	the free lists accordingly.*/
void* buddy_alloc(size_t size);

/* 	Frees the block of memory. Update the free lists accordingly.*/
void buddy_free(void *ptr);

/* 	Invoked by buddy_free or buddy_alloc. Combines blocks of memory 
to a bigger chunk.*/
void* buddy_merge(void *ptr);
