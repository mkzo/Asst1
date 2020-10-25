#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include "mymalloc.h"

#define BLOCK_SIZE 4096
#define META_SIZE 2
static char myblock[BLOCK_SIZE];


/******* Declaring local debugging functions *******/

void print_mem();
void print_block(const void *addr);
void print_bin(const unsigned char *addr);
void error_handler(const char *message, const char *file, int line);


/*********** Getter metadata functions ***********/

/* NOTE: for all metadata functions, *addr should point
   to the start of metadata, NOT the start of a block
*/

/* 
 * Retrieves the used field of metadata, which is the first bit 
 * in *addr.
 * 
 * Parameters
 *     const void *addr - pointer to metadata
 * Preconditions
 *     addr must point to valid metadata
 * Error Handling
 *     None
 * Returns
 *     Value of used (1 or 0)
*/
bool get_used(const void *addr) {
    unsigned char mask = 0x1 << 7;  /* 10000000 in binary, selects first bit */
    unsigned char val = *(unsigned char*)addr; 
    return (mask & val) >> 7;  /* returns value of first bit */
}

/* 
 * Retrieves the size field of metadata, which is split across two 
 * bytes in *addr and *(addr+1).
 * 
 * Parameters
 *     const void *addr - pointer to metadata
 * Preconditions
 *     addr must point to valid metadata
 * Error Handling
 *     None
 * Returns
 *     Value of size (0 <= size < 4096)
*/
size_t get_size(const void *addr) {
    unsigned char *b1 = (unsigned char*)addr;  /* pointers to first and second bytes of metadata */
    unsigned char *b2 = (unsigned char*)addr+1;

    size_t val = 0;
    size_t mask = 0xf; /* 00001111, selects last 4 bits */

    val += ((size_t)(*b1) & mask) << 8;  /* gets first 4 bits of size, cast to uint to prevent overflow */
    val += *b2;  /* b2 last 8 bits, no need for bit manipulation */

    return val;
}

/* 
 * Retrieves the hanging field of metadata, which is located in *addr.
 * 
 * Parameters
 *     const void *addr - pointer to metadata
 * Preconditions
 *     addr must point to valid metadata
 * Error Handling
 *     None
 * Returns
 *     Value of hanging (0, 1, or 2)
*/
size_t get_hanging(const void *addr) {
    unsigned char mask = 0x3 << 4;  /* 00110000 */
    unsigned char val = *(unsigned char*)addr;
    return (mask & val) >> 4;
}

/*********** Setter metadata functions ***********/

/* 
 * Sets the used field of metadata, which is the first bit 
 * in *addr.
 * 
 * Parameters
 *     const void *addr - pointer to metadata
 *     bool used - value of used
 * Preconditions
 *     addr must point to valid metadata
 * Error Handling
 *     None
 * Returns
 *     None
*/
void set_used(const void *addr, bool used) {
    if (get_used(addr) != used) {
        *(unsigned char*)addr ^= (1 << 7);  /* If first bit is different than used, flip it */
    }
}

/* 
 * Sets the size field of metadata, which is split across two 
 * bytes in *addr and *(addr+1).
 * 
 * Parameters
 *     const void *addr - pointer to metadata
 *     size_t size - value of size
 * Preconditions
 *     addr must point to valid metadata
 *     0 < size < 4096
 * Error Handling
 *     If size is out of range, bit masking ensures it doesn't 
 *     modify other parts of metadata
 * Returns
 *     None
*/
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

/* 
 * Sets the hanging field of metadata, which is located in *addr.
 * 
 * Parameters
 *     const void *addr - pointer to metadata
 *     size_t hanging - value of hanging
 * Preconditions
 *     addr must point to valid metadata
 *     0 <= hanging <= 2
 * Error Handling
 *     If hanging is out of range, bit masking ensures it doesn't 
 *     modify other parts of metadata
 * Returns
 *     None
*/
void set_hanging(const void *addr, size_t hanging) {
    unsigned char mask = 0x3 << 4; /* 00110000 */
    unsigned char val = *(unsigned char*)addr;

    val &= ~mask;  /* clear 3rd and 4th bit */
    val |= (mask & (hanging << 4));  /* set target bits to match size */

    *(unsigned char*)addr = val;
}

/*********** Main memory functions ***********/

/* 
 * Allocates a block of memory within the 4096 byte myblock char array and 
 * returns a pointer to allocated memory. Uses first free allocation. Can allocate 
 * a maximum of 4094 bytes.
 * 
 * Parameters
 *     size_t size - size of memory block to allocate
 *     const char* file - file name of caller
 *     int line - line of caller
 * Preconditions
 *     0 < size <= 4094
 * Error Handling
 *     When an error is detected, returns NULL and prints the reason 
 *     for the error with the file name and execution line.
 * 
 *     Terminates if size is out of bounds (0 < size <= 4094) or no space left to allocate.
 * Returns
 *     Pointer to beginning of allocated memory or NULL
 */
void *mymalloc(size_t size, const char* file, int line) {
    /* check size is between 0 and 4096 */
    if (size <= 0 || size > 4094) {
        error_handler("malloc failed (invalid size)", file, line);
        return NULL;
    }

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
                    set_hanging(curr, curr_size-size);
                }
                set_used(curr, 1);
            }

            return curr + META_SIZE;
        }

        /* move to next block */
        curr += curr_size + META_SIZE;
    }

    /* No free blocks large enough */
    error_handler("malloc failed (not enough memory)", file, line);
    return NULL;
}

/* 
 * Frees a block of memory at given address. Steps through memory blocks until it reaches the 
 * given address and attempts to free it. Will merge freed block with adjacent free blocks if 
 * necessary and absorb nearby hanging bytes. Terminates if address is invalid or already freed.
 * 
 * Parameters
 *     const void *addr - pointer to block to be freed
 *     const char* file - file name of caller
 *     int line - line of caller
 * Preconditions
 *     addr must point to the beginning of a block, not its metadata
 * Error Handling
 *     When an error is detected, returns NULL and prints the reason 
 *     for the error with the file name and execution line.
 * 
 *     Terminates if mymalloc() hasn't been called yet.
 *     Terminates if addr points to an invalid address outside of myblock
 *     Terminates if addr doesn't point to the start of a block or an already freed block
 * Returns
 *     None
 */
void myfree(const void *addr, const char* file, int line) {
    /* check if malloc has been called yet */
    if (get_size(myblock) == 0) {
        error_handler("free failed (nothing allocated)", file, line);
        return;
    }

    char *target = (char*)addr;
    /* check if arg is null or outside the range of memory */
    if (target == NULL || !(target >= myblock && target < myblock + BLOCK_SIZE)) {
        error_handler("free failed (invalid pointer)", file, line);
        return;
    }

    /* search for ptr in main memory */
    char *prev = NULL;  /* blocks only point forward, need to keep track of prev block */
    char *curr = myblock;

    while (curr < target) {
        if (curr + META_SIZE == target) {  /* make sure ptr is aligned with start of block */
            /* check if curr is already freed */
            if (get_used(curr) == false) {
                error_handler("free failed (block already freed)", file, line);
                return;
            }

            /* free block */
            set_used(curr, false);
            set_hanging(curr, 0);

            /* Merge with previous memory block if possible */
            if (prev != NULL) {
                /* merge if prev is free*/
                if (get_used(prev) == false) {  
                    size_t new_size = get_size(prev) + get_size(curr) + META_SIZE;
                    set_size(prev, new_size);
                    set_hanging(prev, 0);
                    curr = prev;
                }

                /* try to reclaim hanging memory from prev */
                else if (get_hanging(prev) > 0) { 
                    size_t hanging = get_hanging(prev);

                    set_size(prev, get_size(prev) - hanging);  /* remove hanging from prev */
                    set_hanging(prev, 0);

                    set_size(curr, get_size(curr) + hanging);  /* add mem to curr */

                    *(curr-hanging) = *curr;  /* since curr block gets larger, need to move metadata back */
                    *(curr-hanging+1) = *(curr+1);

                    curr = curr-hanging;  /* move ptr back */
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
    error_handler("free failed (pointer doesn't point to start of block)", file, line);
    return;
}


/*********** Local debugging functions ***********/

void print_mem() {
    char *curr = myblock;
    while (curr < myblock + BLOCK_SIZE) { 
        int used = get_used(curr);
        int size = get_size(curr);
        int hanging = get_hanging(curr);
        printf("(%d, %d, %d) -> ", used, size, hanging);
        curr += get_size(curr) + META_SIZE;
    }
    printf("\n");
}

void print_block(const void *addr) {
    int used = get_used(addr);
    int size = get_size(addr);
    int hanging = get_hanging(addr);

    printf("\nBlock at 0x%p: ", addr);
    print_bin(addr);
    printf("\tUsed: %d\n", used);
    printf("\tSize: %d\n", size);
    printf("\tHanging: %d\n", hanging);
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

void error_handler(const char *message, const char *file, int line) {
    if (file == NULL) {return;}
    printf("mymalloc: %s:%d: %s\n", file, line, message);
}