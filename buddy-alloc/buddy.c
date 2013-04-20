#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "buddy.h"
#define DEBUG 1

static void *mem_base;
static freelist_t *freelist_object;

/* Utility functions */

void print_the_memory_layout() {

    int i=0;
    item *iterator;
    while(i <= freelist_object->max_level) {
        if(freelist_object->freelist[i] == NULL)
            printf("Free blocks at level - %d : 0\n", i);
        else {
            printf("Free blocks at level - %d : ", i);
            iterator = ((item *)freelist_object->freelist[i]);
            printf("%p ", iterator);
            while(iterator->next != NULL) 
                iterator = iterator->next;
            printf("\n");
        }
        ++i;   
    }
}

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

void* find_buddy(void* ptr)
{
    unsigned long buddy, block;

    int level = ((item*)ptr)->level;

    d_printf("Freeing request for a block of level %d\n", level);

    /* Fixup block address to be zero-relative */
    block = ptr - mem_base;

    /* Calculate buddy in zero-relative space */
    buddy = block ^ (1UL << level);

    /* Return the buddy's address */
    return (void *)(buddy + mem_base);
}

/*  Buddy allocation functions */

/*  
    Initialize the freelist object and the array. 
    The arrays' top most index (levels) should point to the 
    entire chunk of memory
*/

void buddy_init() {

    int levels = MAX_LEVELS;
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

/*  
    If the size requested is less than it's next power of 2 we end up in
    internal fragmentation. This method fragments the difference in the 
    requested size and next power of 2 in chunks of powers of 2 and adds 
    them to the appropriate free list levels
*/

void* buddy_exact_alloc(void* ptr, size_t size) {

    d_printf("Internal fragmentation detected : \n");
    int previous_power_of_two, level;
    int current_level = ((item*)ptr)->level;
    int extra_allocated = (1UL << current_level) - size;
    void* block_to_be_freed;
    void* current_block = ptr;
    d_printf("Extra allocated : %d\n", extra_allocated);

    previous_power_of_two = (get_next_power_of_2(extra_allocated) + 1 ) >> 1UL;
    while(previous_power_of_two) {
        
        level = get_level(previous_power_of_two);
        block_to_be_freed = (item *)((unsigned long)current_block + (1UL << level));
        ((item *)current_block)->level = level;
        
        /* Check if there is already free block of that level */
        /* If no, make this the first one */
        if(freelist_object->freelist[level] == NULL)
            freelist_object->freelist[level] = block_to_be_freed;

        /* Else get the first one, link it to this block and make the block the first in that level */
        else {
            ((item *)block_to_be_freed)->next = freelist_object->freelist[level];
            freelist_object->freelist[level] = block_to_be_freed;
        }

        d_printf("Freed and added back, size %d \n", previous_power_of_two);

        current_block = block_to_be_freed;
        extra_allocated = extra_allocated - previous_power_of_two;
        previous_power_of_two = (get_next_power_of_2(extra_allocated) + 1 ) >> 1UL;
    }

    level = get_level(previous_power_of_two);

    // Do the same for last block (level 0, size 1) if present
    if(extra_allocated) {
        if(freelist_object->freelist[level] == NULL)
            freelist_object->freelist[level] = block_to_be_freed;

        /* Else get the first one, link it to this block and make the block the first in that level */
        else {
            ((item *)block_to_be_freed)->next = freelist_object->freelist[level];
            freelist_object->freelist[level] = block_to_be_freed;
        }
        d_printf("Freed and added back, size %d \n", extra_allocated);
    }

    return ptr;
}


void* buddy_alloc(size_t size) {
    
    void* allocated_block = NULL;                  // The object to return
    void* available_block;
    int next_power_of_2 = 0;
    int calculated_level = 0;
    int j = 0;
    void *list = NULL;
    item *new_head;

    d_printf("Allocation request for %d\n", (int)size);

    //Calculate the power of 2 big enough to hold 'size'
    next_power_of_2 = get_next_power_of_2(size);

    calculated_level = get_level(next_power_of_2);

    assert(calculated_level < freelist_object->max_level);

    // Check the free-list if there are any free blocks for that level in the freelist array
    // If not, find a block in the upper level.
    // Split the block
    // Assign the block to the return object.

    for (j = calculated_level; j <= freelist_object->max_level; j++) {

        list = freelist_object->freelist[j];
        if (list == NULL)
            continue;

        available_block = (void *)list;

        /* Detach it from it's current position in the free list array */
        new_head = ((item *)list)->next;

        if(new_head == NULL)
            freelist_object->freelist[j] = NULL;
        else
            freelist_object->freelist[j] = new_head;

        if(available_block == NULL) {
            printf("Memory full. Try evicting\n");
            return NULL;
        }
        d_printf("Big enough block %p found at level %d\n", available_block, j);

        /* Trim if a higher order block than necessary was allocated */
        allocated_block = available_block;

        while (j > calculated_level) {

            /* Perform the splitting iteratively */
            d_printf("Spitting %p at level %d ", available_block, j);
            --j;
            
            available_block = (item *)((unsigned long)available_block + (1UL << j));
            ((item *)available_block)->level = j;
            ((item *)allocated_block)->level = j;
            d_printf("into %p and %p\n", allocated_block, available_block);

            /* Check if there is already free block of that level */
            /* If no, make this the first one */
            if(freelist_object->freelist[j] == NULL)
                freelist_object->freelist[j] = allocated_block;

            /* Else get the first one, link it to this block and make the block the first in that level */
            else {
                ((item *)available_block)->next = freelist_object->freelist[j];
                freelist_object->freelist[j] = allocated_block;
            }
            allocated_block = available_block;
        }

        d_printf("Block %p alloted out of the level %d\n", allocated_block, j);

        // Return the object if it does not cause internal fragmentatation
        if((1UL << calculated_level) - size == 0)
        {
            d_printf("No internal fragmentatation involved with this request size\n");
            return allocated_block;
        }

        //else invoke the buddy_exact_alloc method
        return buddy_exact_alloc(allocated_block, size);
    }

    return NULL;
}




void buddy_free(void *ptr) {

    
    int level, appropriate_level;
    item* block_to_be_freed = (item *)ptr;

    level = block_to_be_freed->level;

    while (level < freelist_object->max_level) {

        // Find the address of the buddy corresponding to this object to be freed
        item* buddy = find_buddy(block_to_be_freed);

        if(buddy == NULL) {
            // Not sure if this check has to be in place. We will always find a buddy, free or not. Right?
            d_printf("No buddy found\n");
            break;
        }
        d_printf("Buddy %p found\n", buddy);

        // Check if it is in the free list already. If yes, merge them.
        // TO-DO

        // Else add the block to be freed in the appropriate level in the freelist.
        appropriate_level = ((item *)block_to_be_freed)->level;

        if(freelist_object->freelist[appropriate_level] == NULL)
            freelist_object->freelist[appropriate_level] = block_to_be_freed;
        else {
            ((item *)block_to_be_freed)->next = freelist_object->freelist[appropriate_level];
            freelist_object->freelist[appropriate_level] = block_to_be_freed;
        }

        /*list_del(&buddy->link);
        if (buddy < block_to_be_freed)
            block_to_be_freed = buddy;*/
        ++level;
        /*block_to_be_freed->level = level;*/
    }

    // No buddy found, so add the block to be freed to its corresponding free list level.

    /*if(freelist_object->freelist[level] == NULL)
        freelist_object->freelist[level] = block_to_be_freed;

     Else get the first one, link it to this block and make the block the first in that level 
    else {
        ((item *)available_block)->next = freelist_object->freelist[level];
        freelist_object->freelist[level] = block_to_be_freed;
    }*/
}