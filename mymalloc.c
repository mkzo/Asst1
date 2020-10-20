#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <math.h>

#define BLOCK_S 30
#define META_S 2
static char myblock[BLOCK_S];

void print_block(char *addr);

// 1 means valid/free, 0 means in use
int meta_get_valid(char *addr) {
    unsigned char mask = 1 << 7;
    unsigned char val = *addr;
    return (mask & val) >> 7;
}

int meta_get_size(char *addr) {
    unsigned char *b1 = addr;
    unsigned char *b2 = addr+1;

    int val = 0;
    val += *b2;

    char mask = 0xf;
    val += ((*b1) & mask) << 8;

    return val;
}

void meta_set_valid(char *addr, int valid) {
    if (meta_get_valid(addr) != valid) {
        *addr ^= 1 << 7;
    }
}

void meta_set_size(char *addr, int size) {
    unsigned char *b1 = addr;
    unsigned char *b2 = addr+1;

    int mask1 = 0xf00;
    int mask2 = 0x0ff;

    // save valid because we will overwrite
    int valid = meta_get_valid(addr);
    *b1 = (size & mask1) >> 8;
    meta_set_valid(addr, valid);

    *b2 = (size & mask2);
}

void *mymalloc(int size) {
    /* Initializes first node, should only run once */
    if (meta_get_size(myblock) == 0) {
        meta_set_valid(myblock, 1);
        meta_set_size(myblock, BLOCK_S - META_S);
    }

    /* Assign new memory block */
    char *nd = myblock;
    while (nd - (char *)myblock < BLOCK_S) { 
        int nd_valid = meta_get_valid(nd);
        int nd_size = meta_get_size(nd);
        /* Free block and enough space to accomodate request */
        if (nd_valid == 1 && nd_size >= size) {
            /* Enough space to make a new block */
            if (nd_size > size + META_S) {
                char *new_nd = nd + (size + META_S);
                meta_set_valid(new_nd, 1);
                meta_set_size(new_nd, nd_size - (size + META_S));

                meta_set_valid(nd, 0);
                meta_set_size(nd, size);

                return nd+META_S;
            }
            /* Just enough space to allocate this block */
            else {
                meta_set_valid(nd, 0);
                return nd+META_S;
            }
        }

        else {
            nd += nd_size + META_S;
        }
    }
    /* Error has occured */
    printf("NO SPACE\n");
    return NULL;
}

void print_mem() {
    for (int i=0; i < META_S; i++) {
        printf("%d ", myblock[i]);
    }
    printf("\n");
}

void print_block(char *addr) {
    int res = meta_get_valid(addr);
    int size = meta_get_size(addr);
    printf("Block at %p:\n\tValid: %d\n\tSize: %d\n\n", addr, res, size);
}

/* For testing purposes, in reality our functions would be included in a library */
int main() {
    // print_block(myblock);

    mymalloc(28);
    print_block(myblock);
    mymalloc(1);

    return 0;
}