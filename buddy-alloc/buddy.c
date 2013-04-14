#include <assert.h>
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
    return size;
}


/* Buddy allocation functions */

void buddy_init() {

    int levels = 13;
    int mem_base_size = 1 << levels;
    int i;

    // Hard code the maximum block of memory for now
    mem_base = malloc(mem_base_size);

    // Iitialize the entire chunk with zeros.
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

    // Initialize the other free list levels using the list_init_link method.
    for ( i = 0; i < levels; ++i)
        freelist_object->freelist[i] = NULL;

    // This should ideally be done with a list utility's insert node function
    freelist_object->freelist[levels] = mem_base;
    
}


void* buddy_alloc(size_t size) {
    
    // The object to return
    void* allocated_block;
    void* available_block;
    int next_power_of_2 = 0;
    int calculated_level = 0;
    int j = 0;
    void *list = NULL;
    item *new_head;

    //Calculate the power of 2 big enough to hold 'size'
    next_power_of_2 = get_next_power_of_2(size);

    calculated_level = get_level(next_power_of_2);

    printf("Calculated Level : %d\n", calculated_level);
    printf("freelist_object->max_level : %d\n", freelist_object->max_level);

    assert(calculated_level < freelist_object->max_level);

    // Check the free-list if there are any free blocks for that level in the freelist array
    // If not, find a block in the upper level.
    // Split the block
    // Assign the block to the return object.

    for (j = calculated_level; j <= freelist_object->max_level; j++) {

        list = freelist_object->freelist[j];
        if (list == NULL)
            continue;

        printf("List : %p\n", list);
        new_head = ((item *)list)->next;
        available_block = (void *)list;
        printf("available_block : %p\n", available_block);

        /* Trim if a higher order block than necessary was allocated */
        printf("J : %d\n", j);
        while (j > calculated_level) {

            /* Perform the splitting iteratively */
            --j;
            allocated_block = (item *)((unsigned long)available_block + (1UL << j));
            ((item *)allocated_block)->level = j;

            if(freelist_object->freelist[j] == NULL)
                freelist_object->freelist[j] = allocated_block;

            else {
                ((item *)allocated_block)->next = freelist_object->freelist[j];
                freelist_object->freelist[j] = allocated_block;
            }
        }

        return allocated_block;
    }

    // Return the object
    return NULL;
}




/*void buddy_free(void *ptr, size_t size) {

    
    int level;

    block_to_be_freed = (item *)ptr;

    level = block_to_be_freed->level;

    while (level < freelist_object->max_level) {

        //Check if there are any free blocks in the freelist level corresponding to size.
        item* buddy = find_buddy(block_to_be_freed);

        if(buddy == NULL)
            break;

        // If yes, check if it is the current block's buddy in the freelist level 
        // corresponding to size. If there is, invoke buddy_merge and merge the two.
        list_del(&buddy->link);
        if (buddy < block_to_be_freed)
            block_to_be_freed = buddy;
        ++level;
        block_to_be_freed->level = level;
    }

    // No buddy found, so add the block to be freed to its corresponding free list level.
    block_to_be_freed->level = level;

    list_add(&block_to_be_freed->link, &freelist_object->freelist[level];);

}*/



int main(int argc, char const *argv[]) {

    void* pointer;
    int i = 0;
    buddy_init();

    // Testing the free list
    for (i = 0; i <= 13; ++i)
        printf("%p\n", freelist_object->freelist[i]);

    pointer = buddy_alloc(19);
    printf("Block allotted : %p\n", pointer);
    // buddy_free(pointer, 9);

    // Testing the free list
    for (i = 0; i <= 13; ++i)
        printf("%p\n", freelist_object->freelist[i]);
    return 0;
}