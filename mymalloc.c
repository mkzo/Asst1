#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include "mymalloc.h"

#define BLOCK_SIZE 4096
#define META_SIZE 2
static char myblock[BLOCK_SIZE];

void error_handler(const char *message, const char *file, int line) {
    if (file == NULL) {return;}
    printf("mymalloc: %s:%d: %s\n", file, line, message);
}

/*********** Retrive metadata functions ***********/

bool get_used(const void *addr) {
    unsigned char mask = 0x1 << 7;  /* 10000000 in binary, selects first bit */
    unsigned char val = *(unsigned char*)addr; 
    return (mask & val) >> 7;
}

size_t get_size(const void *addr) {
    unsigned char *b1 = (unsigned char*)addr;  /* pointers to first and second bytes of metadata */
    unsigned char *b2 = (unsigned char*)addr+1;

    size_t val = 0;
    size_t mask = 0xf; /* 00001111, selects last 4 bits */

    val += ((size_t)(*b1) & mask) << 8;  /* gets first 4 bits of size, cast to uint to prevent overflow */
    val += *b2;  /* b2 last 8 bits, no need for bit manipulation */

    return val;
}

size_t get_freed(const void *addr) {
    unsigned char mask = 0x3 << 4;  /* 00110000 */
    unsigned char val = *(unsigned char*)addr;
    return (mask & val) >> 4;
}

/*********** Store metadata functions ***********/

void set_used(const void *addr, bool used) {
    if (get_used(addr) != used) {
        *(unsigned char*)addr ^= (1 << 7);  /* If first bit is different than used, flip it */
    }
}

void set_size(const void *addr, size_t size) {
    unsigned char *b1 = (unsigned char*)addr;  /* pointers to first and second bytes of metadata */
    unsigned char *b2 = (unsigned char*)addr+1;

    size_t mask1 = 0xf00;  /* 00001111 00000000 */
    size_t mask2 = 0x0ff;  /* 00000000 11111111 */

    
    bool tmp = get_used(addr); /* save first bit in b1 because we will overwrite */
    *b1 = (size & mask1) >> 8;  /* bit shift right to fit in unsigned char */
    set_used(addr, tmp);  /* restore first bit */

    *b2 = (size & mask2);
}

void set_freed(const void *addr, size_t size) {
    unsigned char mask = 0x3 << 4; /* 00110000 */
    unsigned char val = *(unsigned char*)addr;

    val &= ~mask;  /* clear 3rd and 4th bit */
    val |= (mask & (size << 4));  /* set target bits to match size */

    *(unsigned char*)addr = val;
}

/*********** Main memory functions ***********/

void *mymalloc(size_t size, const char* file, int line) {
    /* Initializes first block, runs first time malloc is called */
    /* No block should ever have size 0, only happens when myblock is blank (just initialized) */
    if (get_size(myblock) == 0) {
        set_used(myblock, 0);
        set_size(myblock, BLOCK_SIZE - META_SIZE);
    }

    /* search for free memory block */
    char *curr = myblock;
    while (curr < myblock + BLOCK_SIZE) { 
        size_t curr_used = get_used(curr);
        size_t curr_size = get_size(curr);

        /* Block is free and enough space for request */
        if (curr_used == 0 && curr_size >= size) {
            if (curr_size - size > META_SIZE) {  /* Enough space to split free block into 2 blocks */
                char *new_curr = curr + (size + META_SIZE);
                set_used(new_curr, 0);
                set_size(new_curr, curr_size - (size + META_SIZE));

                set_used(curr, 1);
                set_size(curr, size);
            }

            else {  /* not enough room to make new block */
                if (curr_size > size) {  /* block may have unused bytes at the end */
                    set_freed(curr, curr_size-size);
                }
                set_used(curr, 1);
            }

            return curr + META_SIZE;
        }

        /* move to next block */
        curr += curr_size + META_SIZE;
    }

    /* No free blocks large enough */
    printf("mymalloc: %s:%d: malloc failed (not enough memory)\n", file, line);
    return NULL;
}

void myfree(void *ptr, const char* file, int line) {
    /* check if malloc has been called yet */
    if (get_size(myblock) == 0) {
        printf("mymalloc: %s:%d: free failed (nothing allocated)\n", file, line);
        return;
    }

    char *target = (char*)ptr;
    /* check if arg is null or outside the range of memory */
    if (target == NULL || !(target >= myblock && target < myblock + BLOCK_SIZE)) {
        printf("mymalloc: %s:%d: free failed (invalid pointer)\n", file, line);
        return;
    }

    /* search for ptr in main memory */
    char *prev = NULL;  /* blocks only point forward, need to keep track of prev block */
    char *curr = myblock;

    while (curr < target) {
        if (curr + META_SIZE == target) {  /* make sure ptr is aligned with start of block */
            /* check if curr is already freed */
            if (get_used(curr) == false) {
                printf("mymalloc: %s:%d: free failed (block already freed)\n", file, line);
                return;
            }

            /* free block */
            set_used(curr, false);
            set_freed(curr, 0);

            /* Merge with previous memory block if possible */
            if (prev != NULL) {
                /* merge with prev */
                if (get_used(prev) == false) {  
                    size_t new_size = get_size(prev) + get_size(curr) + META_SIZE;
                    set_size(prev, new_size);
                    set_freed(prev, 0);
                    curr = prev;
                }

                /* try to reclaim memory from prev */
                else if (get_freed(prev) > 0) { 
                    size_t freed = get_freed(prev);

                    set_size(prev, get_size(prev) - freed);  /* remove hanging mem from prev */
                    set_freed(prev, 0);

                    set_size(curr, get_size(curr) + freed);  /* add mem to curr */

                    *(curr-freed) = *curr;  /* since curr block gets larger, need to move metadata back */
                    *(curr-freed+1) = *(curr+1);

                    curr = curr-freed;  /* move ptr back */
                }
            }

            /* Merge with next memory block if possible */
            char *peek = curr + get_size(curr) + META_SIZE; /* address of next block */
            if (peek < (myblock + BLOCK_SIZE) && get_used(peek) == false) {
                size_t new_size = get_size(curr) + get_size(peek) + META_SIZE;
                set_size(curr, new_size);
            }

            return;
        }

        /* advance to next block */
        prev = curr;
        curr += get_size(curr) + META_SIZE;
    }

    /* if block not found, pointer was unaligned */
    printf("mymalloc: %s:%d: free failed (pointer doesn't point to start of block)\n", file, line);
    return;
}

void print_mem() {
    char *curr = myblock;
    while (curr < myblock + BLOCK_SIZE) { 
        int used = get_used(curr);
        int size = get_size(curr);
        int freed = get_freed(curr);
        printf("(%d, %d, %d) -> ", used, size, freed);
        curr += get_size(curr) + META_SIZE;
    }
    printf("\n");
}

void print_block(const void *addr) {
    int used = get_used(addr);
    int size = get_size(addr);
    int freed = get_freed(addr);

    printf("\nBlock at 0x%p: ", addr);
    print_bin(addr);
    printf("\tUsed: %d\n", used);
    printf("\tSize: %d\n", size);
    printf("\tFreed: %d\n", freed);
    printf("\n");
}

void print_bin(const unsigned char *addr) {
    unsigned char val1 = *addr;
    unsigned char val2 = *(addr+1);

    for (int i=7; i >= 0; i--) {
        printf("%d", (val1 >> i) & 1);
    }

    printf(" ");

    for (int i=7; i >= 0; i--) {
        printf("%d", (val2 >> i) & 1);
    }

    printf("\n");
}