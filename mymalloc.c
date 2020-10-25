#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include "mymalloc.h"

#define BLOCK_SIZE 4096
#define META_SIZE 2
static char myblock[BLOCK_SIZE];


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

    size_t mask1 = 0xf00;  // 00001111 00000000
    size_t mask2 = 0x0ff;  // 00000000 11111111

    // save first bit in b1 because we will overwrite
    bool tmp = get_used(addr);
    *b1 = (size & mask1) >> 8;
    set_used(addr, tmp);  /* restore first bit */

    *b2 = (size & mask2);
}

void set_freed(const void *addr, size_t size) {
    unsigned char mask = 0x3 << 4; // 00110000
    unsigned char val = *(unsigned char*)addr;
    val &= ~mask;
    val |= (size << 4);

    *(unsigned char*)addr = val;
}

/*********** Main memory functions ***********/

void *mymalloc(size_t size, const char* file, int line) {
    /* Initializes first node, should only run once */
    if (get_size(myblock) == 0) {
        set_used(myblock, 0);
        set_size(myblock, BLOCK_SIZE - META_SIZE);
    }

    /* search for free memory block */
    char *nd = myblock;
    while (nd < myblock + BLOCK_SIZE) { 
        size_t nd_used = get_used(nd);
        size_t nd_size = get_size(nd);

        /* Free block and enough space to accomodate request */
        if (nd_used == 0 && nd_size >= size) {
            /* Resize blocks */
            if (nd_size - size > META_SIZE) {  /* Enough space to make a new block */
                char *new_nd = nd + (size + META_SIZE);
                set_used(new_nd, 0);
                set_size(new_nd, nd_size - (size + META_SIZE));

                set_used(nd, 1);
                set_size(nd, size);
            }
            else {  /* not enough room to make new block */
                if (nd_size > size) {  /* block may have used bytes at the end */
                    set_freed(nd, nd_size-size);
                }
                set_used(nd, 1);
            }
            return nd + META_SIZE;
        }

        /* move to next block */
        nd += nd_size + META_SIZE;
    }

    /* Error has occured */
    printf("MALLOC FAILED (no memory): in line %d of \"%s\"\n", line, file);
    return NULL;
}

void myfree(void *ptr, const char* file, int line) {
    if (get_size(myblock) == 0) {
        printf("FREE FAILED (nothing allocated): in line %d of \"%s\"\n", line, file);
        return;
    }

    char *target = (char*)ptr;
    if (target == NULL || !(target >= myblock && target < myblock + BLOCK_SIZE)) {
        printf("FREE FAILED (pointer out of bounds): in line %d of \"%s\"\n", line, file);
        return;
    }

    char *prev = NULL;
    char *curr = myblock;
    while (curr < target) {
        if (curr + META_SIZE == target) {
            /* check if curr is already freed */
            if (get_used(curr) == false) {
                printf("FREE FAILED (block already freed): in line %d of \"%s\"\n", line, file);
                return;
            }

            set_used(curr, false);
            set_freed(curr, 0);

            /* Merge with previous memory block if possible */
            if (prev != NULL) {
                if (get_used(prev) == false) {
                    size_t new_size = get_size(prev) + get_size(curr) + META_SIZE;
                    set_size(prev, new_size);
                    set_freed(prev, 0);
                    curr = prev;
                }
                /* try to reclaim memory from prev */
                else if (get_freed(prev) > 0) {
                    size_t freed = get_freed(prev);

                    set_size(prev, get_size(prev) - freed);
                    set_freed(prev, 0);

                    set_size(curr, get_size(curr) + freed);

                    *(curr-freed) = *curr;  /* move metadata back */
                    *(curr-freed+1) = *(curr+1);

                    curr = curr-freed;
                }
            }

            /* Merge with next memory block if possible */
            if (curr + get_size(curr) + META_SIZE < (myblock + BLOCK_SIZE)) {
                /* peek next block */
                char *peek = curr + get_size(curr) + META_SIZE;
                if (get_used(peek) == false) {
                    /* Merge with next memory block */
                    size_t new_size = get_size(curr) + get_size(peek) + META_SIZE;
                    set_size(curr, new_size);
                }
            }
            return;
        }
        prev = curr;
        curr += get_size(curr) + META_SIZE;
    }
    printf("FREE FAILED (pointer doesn't point to block): in line %d of \"%s\"\n", line, file);
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