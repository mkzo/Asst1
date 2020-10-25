#include <time.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/time.h>
#include <assert.h>
#include "mymalloc.h"

/* Workload A */
void workload_a() {
    for (int i = 0; i < 120; i++) {
        char* ptr = malloc(1);
        free(ptr);
    }
}

/* Workload B */
void workload_b() {
    char *arr[120];
    for (int i = 0; i < 120; i++) {
        arr[i] = malloc(1);
    }
    for (int i = 0; i < 120; i++) {
        free(arr[i]);
    }
}

/* Workload C */
void workload_c() {
    srand(time(0)); 

    char *arr[120];
    int alloced = 0, freed = 0;
    while (alloced < 120) {
        if (alloced <= freed) {
            arr[alloced] = malloc(1);
            alloced++;
        }
        else {
            if (rand() % 2) {
                arr[alloced] = malloc(1);
                alloced++;
            }
            else {
                free(arr[freed]);
                freed++;
            }
        }
    }

    while (freed < 120) {
        free(arr[freed]);
        freed++;
    }
}

/* Workload D */
void workload_d() {

    /*****************************/
    /* testing saturating memory */
    /*****************************/

    char *blk[4096];
    int i=0;
    while (i < 4096) {  /* allocate 50 byte blocks until run out of memory */
        blk[i] = mymalloc(50, NULL, 0);  /* calling mymalloc directly with no file suppresses errors */
        if (blk[i] == NULL) {break;}
        i++;
    }

    /* Each block is (50+2) bytes large. 4096//52 = 78, so
    we expect to have 78 blocks before we run out of memory */
    assert(i == 78);

    for (i = i-1; i >= 0; i--) {
        free(blk[i]);
    }


    /*********************************************/
    /* testing insufficient memory/metadata size */
    /*********************************************/

    char *ptr_1 = malloc(4092);  /* allocate a 4092 byte block, block size is 4094 */

    char *ptr_2 = mymalloc(1, NULL, 0);  /* allocate a 1 byte block. since there are only 2 bytes 
                                            left (>= 3 needed), this fails so we suppress errors */

    assert(ptr_2 == NULL);  /* check that malloc failed */
    free(ptr_1);


    /**********************/
    /* testing first free */
    /**********************/

    char *arr[5];

    /* create 3 seperated free blocks of size 30, 20, 10 */
    arr[0] = malloc(30);
    arr[1] = malloc(1);
    arr[2] = malloc(20);
    arr[3] = malloc(1);
    arr[4] = malloc(10);
    free(arr[0]);
    free(arr[2]);
    free(arr[4]);

    char *a = malloc(10);  /* malloc should place this in the first valid block (30) */
    char *b = malloc(20);  /* needs 22 bytes of space, will be placed in arr[2] */

    assert(a == arr[0]);
    assert(b == arr[2]);

    free(a);
    free(b);
    free(arr[1]);
    free(arr[3]);   
}

/* Workload E */
void workload_e() {

    /********************************/
    /* testing memory fragmentation */
    /********************************/

    char *frag[3];
    frag[0] = malloc(100);  /* allocate three blocks adjacent to each other */
    frag[1] = malloc(100);
    frag[2] = malloc(100);

    free(frag[0]);  /* free two edge blocks */
    free(frag[2]);
    free(frag[1]);  /* free middle block last */

    char *ptr = malloc(300);
    assert(frag[0] == ptr);  /* make sure that free combines the three fragmented blocks into 1 free block */
    free(ptr);

    /* initialize 40 blocks of memory */
    char *frag_2[40];
    for (int i=0; i < 40; i++) {
        frag_2[i] = malloc(100);
    }

    /* free all even blocks */
    for (int i=0; i < 40; i += 2) {
        free(frag_2[i]);
    }
    /* free all odd blocks */
    for (int i=1; i < 40; i += 2) {
        free(frag_2[i]);
    }

    ptr = malloc(4094); /* check that the whole memory can be allocated into 1 block */
    assert(ptr != NULL);
    
    free(ptr);

    /**********************************/
    /* testing handling hanging bytes */
    /**********************************/

    char *hang[2];
    hang[0] = malloc(100);
    hang[1] = malloc(100);
    assert(hang[0]+102 == hang[1]);  /* hang[0] and hang[1] should be 102 bytes apart (100 + 2 for metadata) */

    free(hang[0]);  /* free the first block and create a new block, 1 byte smaller */
    hang[0] = malloc(99);   

    /* there is only 1 byte between hang[0] and hang[1], too small for a new block
    to be created inbetween. the "hanging" byte is stored at the end of hang[0] */

    free(hang[1]);  /* free and reallocating hang[1] will use the hanging byte from hang[0] */
    hang[1] = malloc(100);
    
    assert(hang[0]+101 == hang[1]);  /* hang[0] and hang[1] are now 101 bytes apart, because
                                        the hanging bit in hang[0] is used in hang[1] now    */

    free(hang[0]);
    free(hang[1]);
}

/* Takes a function pointer, runs the function 50 times and prints the average running time */
void run_time_recorder(void (*workload_ptr)(), char* str) {
    struct timeval start, end;
    int total = 0;
    for (int i = 0; i < 50; i++) {
        gettimeofday(&start, NULL);
        workload_ptr();
        gettimeofday(&end, NULL);
        total += (end.tv_usec - start.tv_usec);
    }
    printf("%s Average: %d us\n", str, total/50);
}

int main() {   
    /* Declare function pointers to workloads A, B, C, D, E */
    void (*workload_ptr_a)() = &workload_a;
    void (*workload_ptr_b)() = &workload_b;
    void (*workload_ptr_c)() = &workload_c;
    void (*workload_ptr_d)() = &workload_d;
    void (*workload_ptr_e)() = &workload_e;

    /* Run each workload 50 times and print runtime */
    run_time_recorder(workload_ptr_a, "Workload A");
    run_time_recorder(workload_ptr_b, "Workload B");
    run_time_recorder(workload_ptr_c, "Workload C");
    run_time_recorder(workload_ptr_d, "Workload D");
    run_time_recorder(workload_ptr_e, "Workload E");

    return 0;
}