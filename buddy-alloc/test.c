#include "buddy.h"


int main(int argc, char const *argv[]) {

    void *pointer_one;
    buddy_init();

    pointer_one = buddy_alloc(9);
    print_the_memory_layout();
    printf("\n");

    return 0;
}