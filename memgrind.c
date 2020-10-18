#include <time.h>
#include <stdio.h>
#include <stdlib.h>
//#include "mymalloc.h"

/* Workload A */
int workload_a() {
    int i;
    for (i = 0; i < 120; i++) {
        char* a = malloc(1);
        free(a);
    }
    return 0;
}

/* Workload B */
int workload_b() {
    int i;
    char* a[120];
    for (i = 0; i < 120; i++) {
        a[i] = malloc(1);
    }
    for (i = 0; i < 120; i++) {
        free(a[i]);
    }
    return 0;
}

/* Workload C */
int workload_c() {
    int i;
    char* a[120];
    i = 0;
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
    return 0;
}

/* Workload D */
    
/* Workload E */

/* Takes a function pointer, runs the function 50 times and records the average running time */
int run_time_recorder(int *workload_ptr()) {
    int result = -1;
    for (int i = 0; i < 50; i++) {
        result = workload_ptr();
        if (result == -1) {
            printf("Error Detected");
            /* Add errno stuff here */
            return -1;
        }
    }
    return 0;
}

int main() {   
    /* Declare function pointers to workloads A, B, C, D, E */
    int (*workload_ptr_a)() = workload_a;
    int (*workload_ptr_b)() = workload_b;
    int (*workload_ptr_c)() = workload_c;
    /* Run each workload 50 times and print runtime */
    run_time_recorder(workload_ptr_a);
    run_time_recorder(workload_ptr_b);
    run_time_recorder(workload_ptr_c);
    return 0;
}