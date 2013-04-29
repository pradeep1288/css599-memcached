#include "buddy.h"


int main(int argc, char const *argv[]) {

    void *pointer_one, *pointer_two, *pointer_three, *pointer_four;
    buddy_init();
    print_the_memory_layout();
    pointer_one = buddy_alloc(33);
    printf("Allocated block : %p of size : %ld\n", pointer_one, ((item*)pointer_one)->size);
    print_the_memory_layout();
    pointer_two = buddy_alloc(31);
    printf("Allocated block : %p of size : %ld\n", pointer_two, ((item*)pointer_two)->size);
    print_the_memory_layout();
    pointer_three = buddy_alloc(15);
    printf("Allocated block : %p of size : %ld\n", pointer_three, ((item*)pointer_three)->size);
    print_the_memory_layout();
    pointer_four = buddy_alloc(19);
    printf("Allocated block : %p of size : %ld\n", pointer_four, ((item*)pointer_four)->size);
    print_the_memory_layout();
    buddy_free(&pointer_one);
    print_the_memory_layout();
    buddy_free(&pointer_two);
    print_the_memory_layout();
    buddy_free(&pointer_three);
    print_the_memory_layout();
    buddy_free(&pointer_four);
    print_the_memory_layout();
    return 0;
}