/**
 * mini_memcheck
 * CS 241 - Fall 2021
 */
#include <stdio.h>
#include <stdlib.h>



#include "mini_memcheck.h"


int main(int argc, char *argv[]) {
    // Your tests here using malloc and free
    char* a = malloc(10);
    char* b = calloc(20, 1);
    char* c = realloc(a, 30);
    free(c);
    free(b);
    return 0;
}