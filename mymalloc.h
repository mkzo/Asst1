#define malloc(x) mymalloc(x, __FILE__, __LINE__)
#define free(x) myfree(x, __FILE__, __LINE__)
#include <stddef.h>
#include <stdbool.h>

bool get_used(const void *addr);
size_t get_size(const void *addr);
void set_used(const void *addr, bool used);
void set_size(const void *addr, size_t size);

void *mymalloc(size_t size, const char* file, int line);
void myfree(void *ptr, const char* file, int line);

void print_block(const void *addr);
void print_bin(const unsigned char *addr);