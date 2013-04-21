#include "buddy.h"


int main(int argc, char const *argv[]) {

    item *pointer_one;
    buddy_init();

    pointer_one = buddy_alloc(8);
    //print_the_memory_layout();
    buddy_free(pointer_one);
    printf("\n");

    return 0;
}