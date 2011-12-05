#include "my_malloc.h"
#include <string.h>
#include <stdio.h>

int main() {
	
	/*
	 * Test code goes here
	 */
	 
        char *a;
        char b[] = "hello peepz";
        a = my_malloc(sizeof(char)*20);
        strncpy(a, b, 20);

        printf("%s\n", a);
        printf("%s\n", b);

	return 0;
}
