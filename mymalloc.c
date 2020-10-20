#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>

#define SIZE 30
static char myblock[SIZE];




// void* mymalloc(int req) {
//     /* Initializes first node, should only run once */
//     if (first == NULL) {
//         first = (Node*)myblock;
//         first->size = SIZE-sizeof(Node);
//         first->used = false;
//     }
//     /* Assign new memory block */
//     Node* nd = first;
//     while (true) { 
//         /* Free block and enough space to accomodate request */
//         if (nd->used == false && nd->size >= req) {
//             /* Enough space to make a new block */
//             if (nd->size > req + sizeof(Node)) {
//                 Node* new_nd = (Node*)((char*)nd + sizeof(Node) + req);
//                 new_nd->size = nd->size - req -sizeof(Node);
//                 new_nd->used = false;
//                 nd->size = req;
//                 nd->used = true;
//                 printf("%p %p\n", nd, nd+sizeof(Node));
//                 return nd+sizeof(Node);
//             }
//             /* Just enough space to allocate this block */
//             else {
//                 nd->used = true;
//                 return nd+sizeof(Node);
//             }
//         }
//         else {
//             if ((char*)nd + nd->size > (myblock+SIZE-1)) {
//                 printf("Not enough space");
//                 return first;
//             }
//             nd = (Node*)((char*)nd + nd->size + sizeof(Node));
//         }
//     }
//     /* Error has occured */
//     return first;
// }

void print_mem() {
    for (int i=0; i < SIZE; i++) {
        printf("%d ", myblock[i]);
    }
    printf("\n");
}


/* For testing purposes, in reality our functions would be included in a library */
int main() {

    printf("%x\n", log(10));

    return 0;
}