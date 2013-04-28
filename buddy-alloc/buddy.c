#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "buddy.h"
#define DEBUG 0

static void *mem_base;
static freelist_t *freelist_object;

/* Utility functions */

/*
    Utility to print the current layout of the memory
*/
void print_the_memory_layout() {

    int i=0;
    item* iterator;
    item* level_iterator;
    while(i <= freelist_object->max_level) {
        if(freelist_object->freelist[i] == NULL)
            printf("Free blocks at level - %d : 0\n", i);
        else {
            printf("Free blocks at level - %d : ", i);
            iterator = ((item *)freelist_object->freelist[i]);
            printf("[%p] ", iterator);
            level_iterator = iterator->next;
            while(level_iterator != NULL) {
                printf("[%p] ", level_iterator);
                level_iterator = level_iterator->next;
            }
            printf("\n");
        }
        ++i;   
    }
}

/*
    Takes in a power of 2 minus 1 and returns log to the base 2 of that number
*/
int get_level(int x) {

    int count = 0;
    while(x) {
        
        x = x >> 1;
        count+=1;
    }
    return count-=1;
}


/*
    Takes in a number and returns its next power of 2 minus 1
*/
int get_next_power_of_2(int size) {

    --size;
    size |= size >> 1;
    size |= size >> 2;
    size |= size >> 4;
    size |= size >> 8;
    size |= size >> 16;
    return size + 1;
}

/* takes size and returns if its a power of two or not. returns 1 if its a power of two and zero otherwise */
int is_power_of_2(int size)
{
    return (size & (size - 1))? 0 : 1;
}

/*
    Takes in a memory address (item actually) and finds 
    its buddy of the same level
*/
void* find_buddy(void* ptr)
{
    unsigned long buddy, block;

    int size = ((item*)ptr)->size;

    int level = get_level(size);

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

    ((item *)mem_base)->size = mem_base_size;

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

void* buddy_exact_alloc(void** ptr, size_t size) {

    d_printf("Internal fragmentation detected : \n");
    void* actual_pointer = *ptr;

    item* current_block_item = (item*)actual_pointer;
    item* new_current_block_item;
    
    int current_size = current_block_item->size;
    int current_level = get_level(current_size);
    int previous_power_of_two_level, block_size, level;
      
    d_printf("current_level : %d\n", current_level);
    d_printf("current_size : %d\n", current_size);
    
    int extra_allocated = (1UL << current_level) - size;
    d_printf("Extra space allocated : %d\n", extra_allocated);

    // Check if the extra allocated is a power of 2. In that case, we can directly
    // add it to the appropriate level of freelist
    if(extra_allocated == get_next_power_of_2(extra_allocated)) {

        level = get_level(extra_allocated);
        block_size = 1UL << level;
        new_current_block_item = current_block_item + (1UL << level);
        new_current_block_item->size = current_block_item->size - block_size;
        current_block_item->size = block_size;

        /* Check if there is already free block of that level */
        /* If no, make this the first one */

        d_printf("Adding block of size %ld back to the freelist level : %d\n", current_block_item->size, level);
        if(freelist_object->freelist[level] == NULL)
            freelist_object->freelist[level] = (void*)current_block_item;

        /* Else get the first one, link it to this block and make the block the first in that level */
        else {
            current_block_item->next = (item*)freelist_object->freelist[level];
            freelist_object->freelist[level] = (void*)current_block_item;
        }
        return (void*)new_current_block_item;        
    }

    // Else we decompose the extra allocated in powers of 2
    while(extra_allocated) {

        if(extra_allocated == get_next_power_of_2(extra_allocated)) {

            level = get_level(extra_allocated);
            block_size = 1UL << level;
            new_current_block_item = (current_block_item + (1UL << level));
            new_current_block_item->size = current_block_item->size - block_size;
            current_block_item->size = block_size;

            /* Check if there is already free block of that level */
            /* If no, make this the first one */

            d_printf("Adding block of size %ld back to the freelist level : %d\n", current_block_item->size, level);
            if(freelist_object->freelist[level] == NULL)
                freelist_object->freelist[level] = (void*)current_block_item;

            /* Else get the first one, link it to this block and make the block the first in that level */
            else {
                current_block_item->next = (item*)freelist_object->freelist[level];
                freelist_object->freelist[level] = (void*)current_block_item;
            }
            return (void*)new_current_block_item;        
        }

        /*d_printf("Next power of 2 of extra allocated : %d\n", get_next_power_of_2(extra_allocated));
        d_printf("Next power of 2's level of extra allocated : %d\n", get_level(get_next_power_of_2(extra_allocated)));*/
        previous_power_of_two_level = get_level(get_next_power_of_2(extra_allocated));
        --previous_power_of_two_level;
        // d_printf("previous_power_of_two_level: %d\n", previous_power_of_two_level);
        block_size = 1UL << previous_power_of_two_level;

        new_current_block_item = current_block_item + block_size;
        new_current_block_item->size = current_block_item->size - block_size;
        current_block_item->size = block_size;

        /* Check if there is already free block of that level */
        /* If no, make this the first one */
        d_printf("Adding block of size %ld back to the freelist level : %d\n", current_block_item->size, previous_power_of_two_level);
        if(freelist_object->freelist[previous_power_of_two_level] == NULL)
            freelist_object->freelist[previous_power_of_two_level] = (void*)current_block_item;

        /* Else get the first one, link it to this block and make the block the first in that level */
        else {
            current_block_item->next = (item*)freelist_object->freelist[level];
            freelist_object->freelist[previous_power_of_two_level] = (void*)current_block_item;
        }

        current_block_item = new_current_block_item;
        extra_allocated -= block_size;
        // d_printf("extra_allocated : %d\n", extra_allocated);
    }

    return actual_pointer; // The control won't reach here
}


void* buddy_alloc(size_t size) {
    
    void* allocated_block = NULL;                  // The object to return
    item* allocated_block_item = NULL;
    item* available_block_item = NULL;
    int next_power_of_2 = 0;
    int calculated_level = 0;
    int j = 0;
    void *list = NULL;
    item *new_head;

    printf("Allocation request for %d\n", (int)size);

    //Calculate the power of 2 big enough to hold 'size'
    next_power_of_2 = get_next_power_of_2(size);
    calculated_level = get_level(next_power_of_2);

    d_printf("next_power_of_2 %d\n", next_power_of_2);
    d_printf("calculated_level %d\n", calculated_level);

    assert(calculated_level < freelist_object->max_level);

    // Check the free-list if there are any free blocks for that level in the freelist array
    // If not, find a block in the upper level.
    // Split the block
    // Assign the block to the return object.

    for (j = calculated_level; j <= freelist_object->max_level; j++) {

        list = freelist_object->freelist[j];
        if (list == NULL)
            continue;

        available_block_item = (item *)list;

        /* Detach it from it's current position in the free list array */
        new_head = available_block_item->next;

        if(new_head == NULL)
            freelist_object->freelist[j] = NULL;
        else
            freelist_object->freelist[j] = (void*)new_head;

        if(available_block_item == NULL) {
            printf("Memory full. Try evicting\n");
            return NULL;
        }
        d_printf("Big enough block %p found at level %d\n", available_block_item, j);

        /* Trim if a higher order block than necessary was allocated */
        allocated_block_item = available_block_item;

        while (j > calculated_level) {

            /* Perform the splitting iteratively */
            // , (int)(((item *)allocated_block)->size)
            // d_printf("Size : %ld\n", allocated_block_item->size);
            d_printf("Splitting %p at level %d ", available_block_item, j);
            --j;
            
            available_block_item = available_block_item + (1UL << j);
            available_block_item->size = (1UL << j);
            allocated_block_item->size = (1UL << j);
            d_printf("into %p and %p\n", allocated_block_item, available_block_item);

            /* Check if there is already free block of that level */
            /* If no, make this the first one */
            if(freelist_object->freelist[j] == NULL)
                freelist_object->freelist[j] = (void*)allocated_block_item;

            /* Else get the first one, link it to this block and make the block the first in that level */
            else {
                available_block_item->next = (item*)freelist_object->freelist[j];
                freelist_object->freelist[j] = (void*)allocated_block_item;
            }
            allocated_block_item = available_block_item;
        }

        d_printf("Block %p alloted out of the level %d\n", allocated_block_item, j);

        // Return the object if it does not cause internal fragmentatation
        if((1UL << calculated_level) - size == 0)
        {
            d_printf("No internal fragmentatation involved with this request size\n");
            allocated_block = (void*)allocated_block_item;
            return &allocated_block;
        }

        //else invoke the buddy_exact_alloc method
        allocated_block = (void*)allocated_block_item;
        return buddy_exact_alloc(&allocated_block, size);
    }
    return NULL;
}




void buddy_free(void **ptr) {

    void* actual_pointer = *ptr;
    item *item_ptr = (item*)actual_pointer;
    int level = 0;
    printf("item to be freed is: %p\n",item_ptr);
    printf("item to be freed of size %ld\n", item_ptr->size);
    if (is_power_of_2(item_ptr->size))
    {
        level = get_level(item_ptr->size);
        item_ptr->next = (item*)freelist_object->freelist[level];
        freelist_object->freelist[level] = (void*)item_ptr;
        printf("Freed block at address : %p\n", item_ptr);
    }  

    else {
        //handle if it is not a power of two
        printf("TO-DO\n");
    }
}