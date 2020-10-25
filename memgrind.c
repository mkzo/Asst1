#include <time.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
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
    int i;
    char* a;
    a = malloc(4094);
    free(a);
    char* b;
    b = malloc(4096);
    free(b);
    int* c[682];
    char* d; 
    double* e;
    for (i = 0; i < 682; i++) {  //will allocate 682*(4+2) = 4092 bytes
        c[i] = malloc(4);
    }
    for (i = 0; i < 682; i+=2) {  //will free every other 6 byte memory block
        free(c[i]);
    }
    d = malloc(1); //will allocate 1+2 = 3 bytes
    e = malloc(8); //will attempt to allocate 2+8 = 10 bytes, will fail because of memory fragmentation
    return;
}

/* Workload E */
void workload_e() {
    char* a[5];
    for (int i = 0; i < 50; i++) {
        a[0] = malloc(1022);
        a[1] = malloc(1022);
        a[2] = malloc(1022);
        //uses 3*1024 = 3072 bytes
        free(a[0]);
        free(a[2]);
        free(a[1]); //freed memory blocks are now adjacent and should be merged
        a[3] = malloc(4092); //4092 bytes allocated + 2 bytes of metadata uses 4094 bytes
        /* 
        last two free bytes are not sufficient to store the minimum of 1 byte of allocated memory, thus they
        incorporated into a[3]. malloc should be unable to allocate 1 additional byte of data
        */
        a[4] = malloc(1);
        free(a[3]); //frees all 4096 bytes
        a[4] = malloc(1); //allocated 1+2 = 3 bytes
        free(a[4]);
    }
}

/* Takes a function pointer, runs the function 50 times and records the average running time */
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
    run_time_recorder(workload_ptr_d, "Workload D");
    run_time_recorder(workload_ptr_e, "Workload E");
    return 0;
}