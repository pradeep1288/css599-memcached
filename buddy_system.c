#include <stdio.h>
#include <stdlib.h>

typedef void * pointer; /* used for untyped pointers */
#define sizes 4
/* pointers to the free space lists */
pointer freelists[sizes];

/* blocks in freelists[i] are of size 2**i. */
#define BLOCKSIZE(i) (1 << (i))

/* the address of the buddy of a block from freelists[i]. */
#define BUDDYOF(b,i) ((pointer)( ((int)b) ^ (1 << (i)) ))

pointer allocate( int size )
{
     int i;

     /* compute i as the least integer such that i >= log2(size) */
     for (i = 0; BLOCKSIZE( i ) < size; i++);

     if (i >= sizes) {
          printf("no space available");
          return NULL;
     } else if (freelists[i] != NULL) {

          /* we already have the right size block on hand */
          pointer block;
          block = freelists[i];
          freelists[i] = *(pointer *)freelists[i];
          return block;

     } else {

          /* we need to split a bigger block */
          pointer block, buddy;
          block = allocate( BLOCKSIZE( i + 1 ) );

          if (block != NULL) {
              /* split and put extra on a free list */
              buddy = BUDDYOF( block, i );
              *(pointer *)buddy = freelists[i];
              freelists[i] = buddy + sizeof(buddy);
          }

          return block;
     }
}

void deallocate( pointer block, int size )
{
     int i;
     pointer * p;
     pointer buddy;

     /* compute i as the least integer such that i >= log2(size) */
     for (i = 0; BLOCKSIZE( i ) < size; i++);

     /* see if this block's buddy is free */
     buddy = BUDDYOF( block, i );
     p = &freelists[i];
     while ((*p != NULL) && (*p != buddy)) p = (pointer *)*p;

     if (*p != buddy) {

          /* buddy not free, put block on its free list */
          *(pointer *)block = freelists[i];
          freelists[i] = block;

     } else {

          /* buddy found, remove it from its free list */
          *p = *(pointer *)buddy;

          /* deallocate the block and its buddy as one block */
          if (block > buddy) {
              deallocate( buddy, BLOCKSIZE( i + 1 ));
          } else {
              deallocate( block, BLOCKSIZE( i + 1 ));
          }
     }
}

int main(int argc, char const *argv[])
{
    pointer p1;
    freelists[3] = malloc(4 * sizeof(int));
    freelists[2] = malloc( sizeof(int) );
    freelists[1] = malloc( sizeof(int) );
    freelists[0] = malloc( sizeof(int) );

    int d = sizeof(int) / 2;
    printf("Requesting a block of size %d\n", d);
    p1 = allocate( d );
    printf("%p\n", p1);
}
