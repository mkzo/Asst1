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
    char *a = malloc(4094);
    free(a);

    char *b = malloc(4096);
    free(b);
}

/* Workload E */
void workload_e() {
    int *a[1024];
    for (int i = 0; i < 682; i++) {
        a[i] = malloc(4);
    }
    for (int i = 0; i < 682; i+=2) {
        free(a[i]);
    }

    char *b = malloc(1);
    double *c = malloc(8);
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

    /* Run each workload 50 times and print runtime */
    run_time_recorder(workload_ptr_a, "Workload A");
    run_time_recorder(workload_ptr_b, "Workload B");
    run_time_recorder(workload_ptr_c, "Workload C");

    char *c = malloc(1);
    free(c);
    free(c);

    for (int i=0; i < 4100; i++) {
        if (malloc(1) == NULL) {
            printf("%d\n", i);
            break;
        }
    }
    
    return 0;
}