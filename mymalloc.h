//#define malloc(x) mymalloc(x,_FILE_,_LINE_)
//#define free(x) myfree(x,_FILE_,_LINE_)

bool get_used(void *addr);
int get_size(void *addr);
void set_used(void *addr, bool used);
void set_size(void *addr, int size);

void *mymalloc(int size);
void myfree(char *ptr);

void print_block(void *addr);
void print_bin(unsigned char *addr);