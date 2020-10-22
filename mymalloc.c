#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include "mymalloc.h"

#define BLOCK_SIZE 30
#define META_SIZE 2
static char myblock[BLOCK_SIZE];

// 1 means in use, 0 means free
bool get_used(void *addr) {
    unsigned char mask = 0x1 << 7;  // 10000000
    unsigned char val = *(unsigned char*)addr;
    return (mask & val) >> 7;
}

int get_size(void *addr) {
    unsigned char *b1 = (unsigned char*)addr;
    unsigned char *b2 = (unsigned char*)addr+1;

    int val = 0;
    char mask = 0xf;

    val += ((*b1) & mask) << 8;
    val += *b2;

    return val;
}

int get_freed(void *addr) {
    unsigned char mask = 0x3 << 4;  // 00110000
    unsigned char val = *(unsigned char*)addr;
    return (mask & val) >> 4;
}

void set_used(void *addr, bool used) {
    if (get_used(addr) != used) {
        *(unsigned char*)addr ^= 1 << 7;
    }
}

void set_size(void *addr, int size) {
    unsigned char *b1 = (unsigned char*)addr;
    unsigned char *b2 = (unsigned char*)addr+1;

    int mask1 = 0xf00;
    int mask2 = 0x0ff;

    // save valid because we will overwrite
    int valid = get_used(addr);
    *b1 = (size & mask1) >> 8;
    set_used(addr, valid);

    *b2 = (size & mask2);
}

void set_freed(void *addr, int size) {
    unsigned char mask = 0x3 << 4; // 00110000
    
    unsigned char val = *(unsigned char*)addr;
    val &= ~mask;
    val |= (size << 4);

    *(unsigned char*)addr = val;
}

void *mymalloc(int size) {
    /* Initializes first node, should only run once */
    if (get_size(myblock) == 0) {
        set_used(myblock, 0);
        set_size(myblock, BLOCK_SIZE - META_SIZE);
    }

    /* search for free memory block */
    char *nd = myblock;
    while (nd < myblock + BLOCK_SIZE) { 
        int nd_used = get_used(nd);
        int nd_size = get_size(nd);

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

        nd += nd_size + META_SIZE;
    }

    /* Error has occured */
    printf("NO SPACE\n");
    return NULL;
}

void myfree(char *ptr) {
    if (get_size(myblock) == 0) {
        printf("Malloc has not yet been called\n");
        return;
    }
    if (ptr == NULL || !(ptr >= myblock && ptr < myblock + BLOCK_SIZE)) {
        printf("Specified address is not a valid pointer\n");
        return;
    }

    char *prev = NULL;
    char *curr = myblock;
    while (curr < ptr) {
        if (curr + META_SIZE == ptr) {
            printf("Specified pointer found...clearing\n");
            
            set_used(curr, false);
            set_freed(curr, 0);

            /* Merge with previous memory block if possible */
            if (prev != NULL) {
                if (get_used(prev) == false) {
                    int new_size = get_size(prev) + get_size(curr) + META_SIZE;
                    set_size(prev, new_size);
                    set_freed(prev, 0);
                    curr = prev;
                }
                /* try to reclaim memory from prev */
                else if (get_freed(prev) > 0) {
                    int freed = get_freed(prev);

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
                    int new_size = get_size(curr) + get_size(peek) + META_SIZE;
                    set_size(curr, new_size);
                }
            }
            return;
        }
        prev = curr;
        curr += get_size(curr) + META_SIZE;
    }
    printf("Pointer unaligned\n");
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

void print_block(void *addr) {
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

void print_bin(unsigned char *addr) {
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

int main() {
    char *a = mymalloc(10);
    char *b = mymalloc(10);

    print_block(b-2);
    myfree(a);
    print_block(a-2);

    char *c = mymalloc(8);
    print_block(c-2);

    print_mem();

    myfree(b);

    print_mem();

    return 0;
}