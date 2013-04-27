#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "buddy.h"
#define DEBUG 1

static void *mem_base;
static freelist_t *freelist_object;

/* Utility functions */

/*
    Utility to print the current layout of the memory
*/
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
    int previous_power_of_two_level, block_size, level = 0;
    void* actual_pointer = *ptr;
    int current_size = ((item*)actual_pointer)->size;
    int current_level = get_level(current_size);
    d_printf("current_level : %d\n", current_level);
    d_printf("current_size : %d\n", current_size);
    int extra_allocated = (1UL << current_level) - size;
    d_printf("extra_allocated : %d\n", extra_allocated);
    void* new_current_block;
    void* current_block = actual_pointer;

    // Check if the extra allocated is a power of 2. In that case, we can directly
    // add it to the appropriate level of freelist
    if(extra_allocated == get_next_power_of_2(extra_allocated)) {

        level = get_level(extra_allocated);
        d_printf("puh level : %d\n", level);
        new_current_block = (item *)((unsigned long)current_block + (1UL << level));
        ((item *)current_block)->size = extra_allocated;

        /* Check if there is already free block of that level */
        /* If no, make this the first one */
        if(freelist_object->freelist[level] == NULL)
            freelist_object->freelist[level] = current_block;

        /* Else get the first one, link it to this block and make the block the first in that level */
        else {
            ((item *)current_block)->next = freelist_object->freelist[level];
            freelist_object->freelist[level] = current_block;
        }
        return new_current_block;        
    }

    // Else we decompose the extra allocated in powers of 2
    while(extra_allocated) {

        if(extra_allocated == get_next_power_of_2(extra_allocated)) {

            d_printf("extra_allocated : %d\n", extra_allocated);
            level = get_level(extra_allocated);
            d_printf("huh level : %d\n", level);
            new_current_block = (item *)((unsigned long)current_block + (1UL << level));
            ((item *)current_block)->size = extra_allocated;

            /* Check if there is already free block of that level */
            /* If no, make this the first one */
            if(freelist_object->freelist[level] == NULL)
                freelist_object->freelist[level] = current_block;

            /* Else get the first one, link it to this block and make the block the first in that level */
            else {
                ((item *)current_block)->next = freelist_object->freelist[level];
                freelist_object->freelist[level] = current_block;
            }
            return new_current_block;        
        }

        previous_power_of_two_level = get_level(get_next_power_of_2(extra_allocated));
        --previous_power_of_two_level;
        d_printf("previous_power_of_two_level: %d\n", previous_power_of_two_level);
        block_size = 1UL << previous_power_of_two_level;

        new_current_block = (item *)((unsigned long)current_block + block_size);
        ((item *)current_block)->size = block_size;

        /* Check if there is already free block of that level */
        /* If no, make this the first one */
        if(freelist_object->freelist[level] == NULL)
            freelist_object->freelist[level] = current_block;

        /* Else get the first one, link it to this block and make the block the first in that level */
        else {
            ((item *)current_block)->next = freelist_object->freelist[level];
            freelist_object->freelist[level] = current_block;
        }

        current_block = new_current_block;
        extra_allocated -= block_size;
        d_printf("extra_allocated : %d\n", extra_allocated);
    }

    return new_current_block;
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
            // , (int)(((item *)allocated_block)->size)
            d_printf("%d\n", (int)(((item *)allocated_block)->size));
            d_printf("Splitting %p ", available_block);
            d_printf("at level %d ", j);
            --j;
            
            available_block = (item *)((unsigned long)available_block + (1UL << j));
            ((item *)available_block)->size = (1UL << j);
            ((item *)allocated_block)->size = (1UL << j);
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
        // return buddy_exact_alloc(allocated_block, size);
        return allocated_block;
    }
    return NULL;
}




void buddy_free(void *ptr) {

  item *item_ptr = (item*)ptr;
  int level = 0;
  printf("item to be freed is: %p\n",item_ptr);
  printf("item to be freed of size %ld\n", item_ptr->size);
  if (is_power_of_2(2))
  {
     level = get_level(item_ptr->size);
     item_ptr->next = freelist_object->freelist[level];
     freelist_object->freelist[level] = item_ptr;
     printf("freed block of address %p", item_ptr);
  }  

  else {
    //handle if it is not a power of two
  }
}