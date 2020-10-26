memgrind: memgrind.c mymalloc.c mymalloc.h
	gcc memgrind.c mymalloc.c -o memgrind
	
.PHONY: clean
clean:
	-rm memgrind