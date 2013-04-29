#include "buddy.h"


int main(int argc, char const *argv[]) {

    void *pointer_one, *pointer_two, *pointer_three;
    buddy_init(6);
    print_the_memory_layout();
    pointer_one = buddy_alloc(32);
    print_the_memory_layout();
    pointer_two = buddy_alloc(16);
    print_the_memory_layout();
    buddy_free(&pointer_two);
    print_the_memory_layout();
    pointer_three = buddy_alloc(32);
    print_the_memory_layout();
    buddy_free(&pointer_three);
    print_the_memory_layout();
    buddy_free(&pointer_one);
    print_the_memory_layout();
    return 0;
}