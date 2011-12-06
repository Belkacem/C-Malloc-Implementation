#include "my_malloc.h"
#include <string.h>
#include <stdio.h>

int main() {
	
	/*
	 * Test code goes here
	 */
	 
        char *a;
        char b[] = "hello peepz";
        a = my_malloc(sizeof(char)*strlen(b));
        strncpy(a, b, strlen(b));

        printf("%s\n", a);
        printf("%s\n", b);

        my_free(a);

	return 0;
}
