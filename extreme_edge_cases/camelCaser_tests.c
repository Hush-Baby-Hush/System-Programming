/**
 * extreme_edge_cases
 * CS 241 - Fall 2021
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "camelCaser.h"
#include "camelCaser_tests.h"

int test_camelCaser(char **(*camelCaser)(const char *),
                    void (*destroy)(char **)) {
    // TODO: Implement me!

    char **outputs = 0;
    // Empty string
    outputs = (*camelCaser)("");
    if (outputs[0]) {
        destroy(outputs);
        return 0;
    }
    destroy(outputs);

    //Test NULL
    outputs = (*camelCaser)(NULL);
    if (outputs) {
        destroy(outputs);
        return 0;
    }
    destroy(outputs);


    // No punctuation
    outputs = (*camelCaser)("abc ABC");
    if (outputs[0]) {
        destroy(outputs);
        return 0;
    }
    destroy(outputs);

    // Count sentences
    outputs = (*camelCaser)(".....");
    for (int i=0; i<5; i++) {
        if (strcmp(outputs[i], "")) {
            destroy(outputs);
            return 0;
        }
    }
    if (outputs[5] != NULL) {
        destroy(outputs);
        return 0;
    }
    destroy(outputs);


    // One sentence
    outputs = (*camelCaser)("abc ABC.");
    if (strcmp(outputs[0], "abcAbc")) {
        destroy(outputs);
        return 0;
    }
    if (outputs[1]) {
        destroy(outputs);
        return 0;
    }
    destroy(outputs);


    
    // Multiple setences
    outputs = (*camelCaser)(" ABC  123aBC ... a1bc . a2bc   a 1 2b2 C D .b2 3cdef  ABCD aBCD .34a a . b ");
    if (strcmp(outputs[0], "abc123Abc") != 0) {
        destroy(outputs);
        return 0;
    }
    if (strcmp(outputs[1], "") != 0) {
        destroy(outputs);
        return 0;
    }
    if (strcmp(outputs[2], "") != 0) {
        destroy(outputs);
        return 0;
    }
    if (strcmp(outputs[3], "a1bc") != 0) {
        destroy(outputs);
        return 0;
    }

    if (strcmp(outputs[4], "a2bcA12B2CD") != 0) {
        destroy(outputs);
        return 0;
    }

    if (strcmp(outputs[5], "b23CdefAbcdAbcd") != 0) {
        destroy(outputs);
        return 0;
    }


    if (strcmp(outputs[6], "34aA") != 0) {
        destroy(outputs);
        return 0;
    }


    if (outputs[7] != NULL) {
        destroy(outputs);
        return 0;
    }
    destroy(outputs);




    // ASCII chars 
    outputs = (*camelCaser)(" \a   \b \n     \t     ");
    if (outputs[0] != NULL) {
        destroy(outputs);
        return 0;
    }
    destroy(outputs);

    outputs = (*camelCaser)("Hello\x1fWorld.");
    if (strcmp(outputs[0], "hello\x1fworld") != 0) {
        destroy(outputs);
        return 0;
    }
    if (outputs[1] != NULL) {
        destroy(outputs);
        return 0;
    }
    destroy(outputs);


    outputs = (*camelCaser)("\a\bas\a\bas, f62 and \a y,");
    if (strcmp(outputs[0], "\a\bas\a\bas") != 0) {
        destroy(outputs);
        return 0;
    }
    if (strcmp(outputs[1], "f62And\aY") != 0) {
        destroy(outputs);
        return 0;
    }

    if (outputs[2] != NULL) {
        destroy(outputs);
        return 0;
    }
    destroy(outputs);


    outputs = (*camelCaser)(" qwr \v \af, LoveU  ");
    if (strcmp(outputs[0], "qwr\aF") != 0) {
        destroy(outputs);
        return 0;
    }
    if (outputs[1] != NULL) {
        destroy(outputs);
        return 0;
    }
    destroy(outputs);


    outputs = (*camelCaser)("54\v9 IQ\\T : M\bE q78pE\nm Gu\ni?");
    if (strcmp(outputs[0], "549Iq") != 0) {
        destroy(outputs);
        return 0;
    }
    if (strcmp(outputs[1], "t") != 0) {
        destroy(outputs);
        return 0;
    }
    if (strcmp(outputs[2], "m\beQ78peMGuI") != 0) {
        destroy(outputs);
        return 0;
    }
    if (outputs[3] != NULL) {
        destroy(outputs);
        return 0;
    }    
    destroy(outputs);

    // TODO: Return 1 if the passed in function works properly; 0 if it doesn't.
    return 1;

}
