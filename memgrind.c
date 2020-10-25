#include <time.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
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
    char *a = malloc(4094);
    free(a);

    char *b = malloc(4096);
    free(b);

    int *c[682];
    for (int i = 0; i < 682; i++) {  // will allocate 682*(4+2) = 4092 bytes
        c[i] = malloc(4);
    }
    for (int i = 0; i < 682; i+=2) {  // will free every other 6 byte memory block
        free(c[i]);
    }

    char *d = malloc(1); // will allocate 1+2 = 3 bytes
    double *e = malloc(8); // will attempt to allocate 2+8 = 10 bytes, will fail because of memory fragmentation
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


    /*********************************************/
    /* testing insufficient memory/metadata size */
    /*********************************************/

    char *ptr_1 = malloc(4092);  /* allocate a 4092 byte block, block size is 4094 */

    /* calling mymalloc directly with no filename suppresses error messages */
    char *ptr_2 = mymalloc(1, NULL, 0);  /* allocate a 1 byte block. since there are only 2 
                                            bytes left (>= 3 needed), this should fail */

    assert(ptr_2 == NULL);  /* check that malloc failed */

    free(ptr_1);
}

/* Takes a function pointer, runs the function 50 times and prints the average running time */
void run_time_recorder(void (*workload_ptr)(), char* str) {
    double total = 0;
    for (int i = 0; i < 5000; i++) {
        clock_t start = clock();
        workload_ptr();
        clock_t end = clock();
        total += (double)(end - start) / CLOCKS_PER_SEC;
    }
    printf("%s Average: %lf us\n", str, total/5000);
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
    // run_time_recorder(workload_ptr_d, "Workload D");
    run_time_recorder(workload_ptr_e, "Workload E");
    return 0;
}