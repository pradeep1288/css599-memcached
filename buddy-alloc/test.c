#include "buddy.h"


int main(int argc, char const *argv[]) {

    void *pointer_one, *pointer_two, *pointer_three;
    buddy_init();

    pointer_one = buddy_alloc(33);
    printf("Allocated block : %p of size : %ld\n", pointer_one, ((item*)pointer_one)->size);
    pointer_two = buddy_alloc(31);
    printf("Allocated block : %p of size : %ld\n", pointer_two, ((item*)pointer_two)->size);
    pointer_three = buddy_alloc(15);
    printf("Allocated block : %p of size : %ld\n", pointer_three, ((item*)pointer_three)->size);
    //buddy_free(&pointer_one);
    print_the_memory_layout();

    return 0;
}