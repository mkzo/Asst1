#define malloc(x) mymalloc(x)
#define free(x) myfree(x)
#include <stddef.h>
#include <stdbool.h>

bool get_used(const void *addr);
int get_size(const void *addr);
void set_used(const void *addr, bool used);
void set_size(const void *addr, size_t size);

void *mymalloc(size_t size);
void myfree(void *ptr);

void print_block(const void *addr);
void print_bin(const unsigned char *addr);