#include "buddy.h"


int main(int argc, char const *argv[]) {

    void *pointer_one, *pointer_two, *pointer_three;
    buddy_init();

    pointer_one = buddy_alloc(19);
    print_the_memory_layout();
    printf("\n");
    pointer_two = buddy_alloc(9);
    /*printf("Block allotted : %p\n", pointer_one);
    printf("Block allotted : %p\n", pointer_two);*/
    // buddy_free(pointer, 9);
    
    print_the_memory_layout();
    printf("\n");
    pointer_three = buddy_alloc(13);
    print_the_memory_layout();
    printf("\n");
    buddy_free(pointer_three);
    print_the_memory_layout();
    printf("\n");
    /*// Testing the free list
    for (i = 0; i <= 13; ++i)
        printf("%p\n", freelist_object->freelist[i]);*/
    return 0;
}