#include "buddy.h"


int main(int argc, char const *argv[]) {

    void *pointer_one, *pointer_two, *pointer_three, *pointer_four, *pointer_five, *pointer_six;
    buddy_init(13);
    print_the_memory_layout();
    pointer_one = buddy_alloc(128);
    printf("Allocated block : %p of size : %ld\n", pointer_one, ((item*)pointer_one)->size);
    print_the_memory_layout();
    pointer_two = buddy_alloc(256);
    printf("Allocated block : %p of size : %ld\n", pointer_two, ((item*)pointer_two)->size);
    print_the_memory_layout();
    pointer_three = buddy_alloc(512);
    printf("Allocated block : %p of size : %ld\n", pointer_three, ((item*)pointer_three)->size);
    print_the_memory_layout();
    pointer_four = buddy_alloc(1024);
    printf("Allocated block : %p of size : %ld\n", pointer_four, ((item*)pointer_four)->size);
    print_the_memory_layout();
    pointer_five = buddy_alloc(2048);
    printf("Allocated block : %p of size : %ld\n", pointer_five, ((item*)pointer_five)->size);
    print_the_memory_layout();
    pointer_six = buddy_alloc(4096);
    printf("Allocated block : %p of size : %ld\n", pointer_six, ((item*)pointer_six)->size);
    print_the_memory_layout();
    buddy_free(&pointer_one);
    print_the_memory_layout();
    buddy_free(&pointer_two);
    print_the_memory_layout();
    buddy_free(&pointer_three);
    print_the_memory_layout();
    buddy_free(&pointer_four);
    print_the_memory_layout();
    buddy_free(&pointer_five);
    print_the_memory_layout();
    buddy_free(&pointer_six);
    print_the_memory_layout();    
    return 0;
}