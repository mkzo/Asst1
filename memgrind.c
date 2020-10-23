#include <time.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include "mymalloc.h"

/* Workload A */
void workload_a() {
    for (int i = 0; i < 120; i++) {
        char* a = malloc(1);
        free(a);
    }
}

/* Workload B */
void workload_b() {
    char* a[120];
    for (int i = 0; i < 120; i++) {
        a[i] = malloc(1);
    }
    for (int i = 0; i < 120; i++) {
        free(a[i]);
    }
}

/* Workload C */
void workload_c() {
    char* a[120];
    int i = 0;
    int j = 0;
    srand(time(NULL)); 
    for (int k = 0; k < 240; k++) {
        if (i == 120) {
            break;
        }
        else if (i <= j) {
            a[i++] = malloc(1);
        }
        else {
            if (rand() % 2) {
                a[i++] = malloc(1);
            }
            else {
                free(a[j++]);
            }
        }
    }
    while (j < 120) {
        free(a[j++]);
    }
}

/* Workload D */
    
/* Workload E */
 
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
    
    return 0;
}