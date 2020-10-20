#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>

/* Node size is 12 bytes */
typedef struct Node {
    int magicNumber;
    int size;
    bool used;
} Node;

static char myblock[4096];
/* Global Linked List Head */
static Node* first;

void* mymalloc(int req) {
    /* Initializes first node, should only run once */
    if (first == NULL) {
        first = (Node*)myblock;
        first->magicNumber = 329851398;
        first->size = 4096-sizeof(Node);
        first->used = false;
    }
    /* Assign new memory block */
    Node* nd = first;
    while (true) { 
        /* Free block and enough space to accomodate request */
        if (nd->used == false && nd->size >= req) {
            /* Enough space to make a new block */
            if (nd->size > req + sizeof(Node)) {
                Node* new_nd = (Node*)((char*)nd + sizeof(Node) + req);
                new_nd->size = nd->size - req -sizeof(Node);
                new_nd->used = false;
                new_nd->magicNumber = 329851398;
                nd->size = req;
                nd->used = true;
                return (char*)nd+sizeof(Node);
            }
            /* Just enough space to allocate this block */
            else {
                nd->used = true;
                return (char*)nd+sizeof(Node);
            }
        }
        else {
            if ((char*)nd + nd->size > (myblock+4095)) {
                printf("Not enough space");
                return (char*)first;
            }
            printf("Checking next block\n");
            nd = (Node*)((char*)nd + nd->size + sizeof(Node));
        }
    }
    /* Error has occured */
    return (char*)first;
}

void myfree(void* ptr) {
    if (first == NULL) {
        printf("Malloc has not yet been called");
        return;
    }
    if (ptr == NULL) {
        printf("Specified address is not a pointer");
        return;
    }
    Node* nd1 = first;
    Node* nd2 = first;
    while ((char*) nd2 < (char*) ptr) {
        printf("nd %p, %p\n", nd2, ptr);
        // if ((char*)nd2 + sizeof(Node) + nd2->size > (myblock+4095)) {
        //         printf("Specified pointer has not been malloced");
        //         return;
        //     }
        if ((char*) nd2 + sizeof(Node) == (char*) ptr) {
            printf("Specified pointer found...clearing\n");
            /* Free procedure */
            nd2->used = false;
            if (nd1->used == false) {
                /* Merge with previous memory block if possible */
                nd1->size = nd1->size + nd2->size + sizeof(Node);
            }
            nd1 = nd2; // Hopefully this doesn't translate changes to nd2 into nd1 too
            if ((char*)nd2 + nd2->size <= (myblock+4095)) {
                /* If possible, increment nd2 to the next memory block */
                nd2 = (Node*)((char*)nd2 + nd2->size + sizeof(Node));
                if (nd2->used == false) {
                    /* Merge with next memory block */
                    nd1->size = nd1->size + nd2->size + sizeof(Node);
                }
            }
            return;
        }
        else if ((char*) nd2 > (char*) ptr) {
            printf("Specified pointer has not been malloced\n");
            return;
        }
        else {
            printf("Checking next block\n");
                if (nd2 == first) {
                    nd1 = first;
                }
                else {
                    nd1 = (Node*)((char*)nd1 + nd1->size + sizeof(Node));
                }
                nd2 = (Node*)((char*)nd2 + nd2->size + sizeof(Node));
        }
    }
    return;
}

/* For testing purposes, in reality our functions would be included in a library */
int main() {
    printf("%p\n",myblock);
    char* c = mymalloc(10);
    printf("%p\n",c);
    char* d = mymalloc(10);
    printf("%p\n",d);
    myfree(c);
    c = mymalloc(10);
    printf("%p\n",c);
    return 0;
}