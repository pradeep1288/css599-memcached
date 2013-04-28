#include "buddy.h"


int main(int argc, char const *argv[]) {

    void *pointer_one;
    buddy_init();

    pointer_one = buddy_alloc(61);
    //print_the_memory_layout();
    printf("Allocated block : %p of size : %ld\n", pointer_one, ((item*)pointer_one)->size);
    buddy_free(&pointer_one);
    printf("\n");

    return 0;
}