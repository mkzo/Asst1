#ifndef MYMALLOC_H
#define MYMALLOC_H

#define malloc(x) mymalloc(x, __FILE__, __LINE__)
#define free(x) myfree(x, __FILE__, __LINE__)
#include <stddef.h>
#include <stdbool.h>

bool get_used(const void *addr);
size_t get_size(const void *addr);
size_t get_hanging(const void *addr);

void set_used(const void *addr, bool used);
void set_size(const void *addr, size_t size);
void set_hanging(const void *addr, size_t size);

void *mymalloc(size_t size, const char* file, int line);
void myfree(const void *addr, const char* file, int line);

#endif