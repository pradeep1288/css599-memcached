#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "buddy.h"
#include "list.h"

static void *mem_base;
static freelist_t *freelist_object;

/* Utility functions */


int get_level(int x) {

    int count = 0;
    while(x) {
        x = x >> 1;
        count++;
    }
    return count;
}


int get_next_power_of_2(int size) {

    --size;
    size |= size >> 1;
    size |= size >> 2;
    size |= size >> 4;
    size |= size >> 8;
    size |= size >> 16;
    return size + 1;
}

/* Buddy allocation functions */

void buddy_init() {

    int levels = 13;
    int mem_base_size = 1<<levels;
    int i = 0;

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

    // Allocate memory for freelist_t struct
    freelist_object = (freelist_t *) malloc (sizeof(freelist_t));

    // Assign the maximum level to freelist_t objects max_level member
    freelist_object->max_level = levels;

    // Allocate memory for freelist array within this struct
    freelist_object->freelist = malloc(levels * sizeof(void *));

    // This should ideally be done with a list utility's insert node function
    freelist_object->freelist[levels] = mem_base;

    // Initialize the other free list levels using the list_init_link method.
    for ( i = 0; i < levels; ++i) {
        //Invoke the list init method on freelist_object->freelist[i]
    }
}

void* buddy_alloc(size_t size) {
    
    // The object to return
    void* allocated_object;
    int next_power_of_2 = 0;
    int calculated_level = 0;
    int j = 0;
    struct list_head *list;

    //Calculate the power of 2 big enough to hold 'size'
    next_power_of_2 = get_next_power_of_2(size);

    calculated_level = get_level(next_power_of_2);

    assert(calculated_level > freelist_object->max_level);

    // Check the free-list if there are any free blocks for that level in the freelist array
    // If not, find a block in the upper level.
    // Split the block
    // Assign the block to the return object.

    for (j = calculated_level; j <= freelist_object->max_level; j++) {

        list = &freelist_object->freelist[j];
        if (list_empty(list))
            continue;

        allocated_object = list_entry(list->next);

        /* Trim if a higher order block than necessary was allocated */
        while (j > order) {
            
            buddy_split(allocated_object);

            // TO-DO : Add it's buddy to the corresponding level in the freelist array

        }

        return allocated_object;
    }

    // Return the object
    return NULL;
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