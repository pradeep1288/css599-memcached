#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "buddy.h"

static void *mem_base;
static freelist_t *freelist_object;

void buddy_init() {

    int levels = 13;
    int mem_base_size = 1<<levels;

    // Hard code the maximum block of memory for now
    mem_base = malloc(mem_base_size);

    // Iitialize the entire chunk with zeros. Initialize the freelist array
    if (mem_base != NULL)
        memset(mem_base, 0, sizeof(mem_base));

    else {
        printf("buddy.c : Error allocating memory\n");
        exit(1);
    }

    // Initialize the freelist object and the array. 
    // The arrays' top most index (levels) should point to the 
    // entire chunk of memory
    freelist_object = (freelist_t *) malloc (sizeof(freelist_t));
    freelist_object->freelist = malloc(levels * sizeof(void *));
    freelist_object->freelist[levels] = mem_base;
}

void* buddy_alloc(size_t size) {
    
    // The object to return
    void* allocated_object;

    //Calculate the power of 2 big enough to hold 'size'

    // Check the free-list if there are any free blocks for that level in the freelist array

    // If not, find a block in the upper level.

    // Split the block

    // Assign the block to the return object.

    // Add it's buddy to the corresponding level in the freelist array

    // Return the object
    return allocated_object;
}

void buddy_free(void *ptr, size_t size) {

    //Check if there are any free blocks in the freelist level corresponding to size.

    // If yes, check if it is the current block's buddy in the freelist level 
    // corresponding to size. If there is, invoke buddy_merge and merge the two.

    // Else, add it to the freelist level corresponding to size.
}

int main(int argc, char const *argv[]) {

    buddy_init();
    buddy_alloc(9);
    // buddy_free(9);
    return 0;
}