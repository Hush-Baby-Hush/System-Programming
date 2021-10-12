/**
 * perilous_pointers
 * CS 241 - Fall 2021
 */
#include "part2-functions.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/**
 * (Edit this function to print out the "Illinois" lines in
 * part2-functions.c in order.)
 */
int main() {
    // your code here

    //first_step
    first_step(81);

    //second_step
    int* i = (int*) calloc(2, sizeof(int));
    *i = 132;
    second_step(i);
    free(i);

    //double_step
    int** db = (int**) calloc(2, sizeof(int*));
    db[0] = (int*) calloc(2, sizeof(int));
    *db[0] = 8942;
    double_step(db);
    free(db[0]);
    free(db);

    //strange_step
    char* value = (char*) calloc(10, sizeof(char));
    int* strange = (int*)(value + 5);
    *strange = 15;
    strange_step(value);
    free(value);

    //empty_step
    void* val = (void*) calloc(5, sizeof(void));
    ((char*)val)[3] = 0;
    empty_step(val);
    free(val);

    //two_step
    char *s2 = (char*) calloc(5, sizeof(char));
    void *s = (void*) s2;
    s2[3] = 'u';
    two_step(s,s2);
    free(s2);

    //three_step
    char *first="abcd";
    char *second = first+2;
    char *third = second+2;
    three_step(first, second, third);

    //step_step_step
    char *fst = (char*) calloc(5, sizeof(char));
    char *snd = (char*) calloc(5, sizeof(char));
    char *trd = (char*) calloc(5, sizeof(char));
    snd[2] = fst[1]+8;
    trd[3] = snd[2]+8;
    step_step_step(fst,snd,trd);
    free(trd);
    free(snd);
    free(fst);

    //it_may_be_odd
    int b = 3;
    char *a = (char*) calloc(6, sizeof(char));
    *a = b;
    it_may_be_odd(a, b);
    free(a);

    //tok_step
    char str[15] = "abcd,CS241";
    tok_step(str);
    
    //the_end
    void *blue = (void*) calloc(6, sizeof(void));
    void *orange = blue;
    ((char *)blue)[0] =1;
    ((char *)blue)[1] =1;
    ((char *)blue)[2] =1;
    ((char *)blue)[3] =0;
    the_end(orange, blue);
    free(blue);

    

    return 0;
}
