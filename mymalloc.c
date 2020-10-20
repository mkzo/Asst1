#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <math.h>

#define BLOCK_SIZE 2000
#define META_SIZE 2
static char myblock[BLOCK_SIZE];

void print_block(char *addr);


// 1 means in use, 0 means free
int get_used(char *addr) {
    unsigned char mask = 1 << 7;  // 10000000
    unsigned char val = *addr;
    return (mask & val) >> 7;
}

int get_size(char *addr) {
    unsigned char *b1 = addr;
    unsigned char *b2 = addr+1;

    int val = 0;
    char mask = 0xf;
    val += ((*b1) & mask) << 8;
    val += *b2;
    return val;
}

void set_used(char *addr, int used) {
    if (get_used(addr) != used) {
        *addr ^= 1 << 7;
    }
}

void set_size(char *addr, int size) {
    unsigned char *b1 = addr;
    unsigned char *b2 = addr+1;

    int mask1 = 0xf00;
    int mask2 = 0x0ff;

    // save valid because we will overwrite
    int valid = get_used(addr);
    *b1 = (size & mask1) >> 8;
    set_used(addr, valid);

    *b2 = (size & mask2);
}

void *mymalloc(int size) {
    /* Initializes first node, should only run once */
    if (get_size(myblock) == 0) {
        set_used(myblock, 0);
        set_size(myblock, BLOCK_SIZE - META_SIZE);
    }

    /* Assign new memory block */
    char *nd = myblock;
    while (nd - (char *)myblock < BLOCK_SIZE) { 
        int nd_used = get_used(nd);
        int nd_size = get_size(nd);
        /* Free block and enough space to accomodate request */
        if (nd_used == 0 && nd_size >= size) {
            /* Enough space to make a new block */
            if (nd_size > size + META_SIZE) {
                char *new_nd = nd + (size + META_SIZE);
                set_used(new_nd, 0);
                set_size(new_nd, nd_size - (size + META_SIZE));

                set_used(nd, 1);
                set_size(nd, size);

                return nd+META_SIZE;
            }
            /* Just enough space to allocate this block */
            else {
                set_used(nd, 1);
                return nd + META_SIZE;
            }
        }

        else {
            nd += nd_size + META_SIZE;
        }
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
            /* Merge with previous memory block if possible */
            if (prev != NULL && get_used(prev) == false) {
                int new_size = get_size(prev) + get_size(curr) + META_SIZE;
                set_size(prev, new_size);
                curr = prev;
            }

            if (curr + get_size(curr) < (myblock + BLOCK_SIZE)) {
                /* If possible, increment nd2 to the next memory block */
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
    for (int i=0; i < META_SIZE; i++) {
        printf("%d ", myblock[i]);
    }
    printf("\n");
}

void print_block(char *addr) {
    int res = get_used(addr);
    int size = get_size(addr);
    printf("Block at %p:\n\tUsed: %d\n\tSize: %d\n\n", addr, res, size);
}

/* For testing purposes, in reality our functions would be included in a library */
// int main() {
//     // print_block(myblock);

//     mymalloc(10);
//     print_block(myblock);
//     mymalloc(10);

//     myfree(myblock+2);
//     print_block(myblock);

//     return 0;
// }
int main() {
    printf("MY BLOCK: %p\n",myblock);

    char* c = mymalloc(10);
    printf("%p\n",c);

    char* d = mymalloc(10);
    printf("%p\n",d);

    char* e = mymalloc(10);
    printf("%p\n",e);

    char* f = mymalloc(10);
    printf("%p\n",f);

    myfree(c);
    myfree(e);
    myfree(d);

    print_block(myblock);

    c = mymalloc(30);
    d = mymalloc(10);
    printf("%p\n",c);
    printf("%p\n",d);

    return 0;
}