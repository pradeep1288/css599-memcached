#include "buddy.h"


int main(int argc, char const *argv[]) {

    void *pointer_one, *pointer_two, *pointer_three, *pointer_four;
    void *pointer5, *pointer6, *pointer7, *pointer8, *pointer9, *pointer10, *pointer11;
    buddy_init();

    pointer_one = buddy_alloc(33);
    printf("Allocated block : %p of size : %ld\n", pointer_one, ((item*)pointer_one)->size);
    pointer_two = buddy_alloc(31);
    printf("Allocated block : %p of size : %ld\n", pointer_two, ((item*)pointer_two)->size);
    pointer_three = buddy_alloc(15);
    printf("Allocated block : %p of size : %ld\n", pointer_three, ((item*)pointer_three)->size);
    pointer_four = buddy_alloc(19);
    printf("Allocated block : %p of size : %ld\n", pointer_four, ((item*)pointer_four)->size);
    pointer5 = buddy_alloc(128);
    printf("Allocated block : %p of size : %ld\n", pointer5, ((item*)pointer5)->size);
    pointer6 = buddy_alloc(256);
    printf("Allocated block : %p of size : %ld\n", pointer6, ((item*)pointer6)->size);
    pointer7 = buddy_alloc(512);
    printf("Allocated block : %p of size : %ld\n", pointer7, ((item*)pointer7)->size);
    pointer8 = buddy_alloc(1024);
    printf("Allocated block : %p of size : %ld\n", pointer8, ((item*)pointer8)->size);
    pointer9 = buddy_alloc(2048);
    printf("Allocated block : %p of size : %ld\n", pointer9, ((item*)pointer9)->size);
    pointer10 = buddy_alloc(4096);
    printf("Allocated block : %p of size : %ld\n", pointer10, ((item*)pointer10)->size);
    pointer11 = buddy_alloc(8);
    print_the_memory_layout();
    buddy_free(&pointer_one);
    //test_buddy();
    buddy_free(&pointer6);
    buddy_free(&pointer8);
    buddy_free(&pointer11);
    buddy_free(&pointer_four);
    print_the_memory_layout();

    return 0;
}