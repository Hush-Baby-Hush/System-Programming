/**
 * vector
 * CS 241 - Fall 2021
 */
#include "sstring.h"
#include "vector.h"

#ifndef _GNU_SOURCE
#define _GNU_SOURCE
#endif

#include <assert.h>
#include <string.h>

struct sstring {
    // Anything you want
    char *s;
    size_t len;

};

sstring *cstr_to_sstring(const char *input) {
    // your code goes here
    sstring *ss = calloc(1,sizeof(sstring));
    ss->s = calloc(strlen(input)+1,sizeof(char));
    // ss->s[strlen(input)]='\0';
    // for (int i = 0; i < strlen(input); i++)
    // {
    //     ss->s[i] = input[i];
    // }
    strcpy(ss->s,input);
    ss->len = strlen(input);

    return ss;
}

char *sstring_to_cstr(sstring *input) {
    // your code goes here
    char *ss = calloc(input->len+1,sizeof(char));
    strcpy(ss,input->s);
    return ss;
}

int sstring_append(sstring *this, sstring *addition) {
    // your code goes here
    this->s = realloc(this->s,this->len+addition->len+1);
    strcpy(this->s+this->len,addition->s);
    this->len +=addition->len;
    return this->len;
}

vector *sstring_split(sstring *this, char delimiter) {
    // your code goes here
    vector *vec = vector_create(string_copy_constructor,string_destructor,string_default_constructor);

    char *next;
    char *curr = this->s;
    while ((next = strchr(curr, delimiter)) != NULL) 
    {
        /* process curr to next-1 */
        char *temp = next;
        char temp2 = *next;
        *next = '\0';
        vector_push_back(vec, curr);
        *next = temp2;
        curr = temp+1;   
    }
/* process the remaining string (the last token) */
    vector_push_back(vec, curr);   
    return vec;
}

int sstring_substitute(sstring *this, size_t offset, char *target,
                       char *substitution) {
    // your code goes here
    if (offset>this->len)
    {
        return -1;
    }

    char* next = strstr(this->s+offset, target);
    if (next)
    {
        char* ss = calloc(this->len+strlen(substitution)-strlen(target)+1,sizeof(char));
        strncpy(ss,this->s,next-this->s);
        strcpy(ss+(next-this->s), substitution);
        strcpy(ss+(next-this->s)+strlen(substitution), next+strlen(target));
        free(this->s);
        this->s = ss;
        this->len = strlen(ss);
        return 0;
    }
    return -1;
}

char *sstring_slice(sstring *this, int start, int end) {
    char * slice = calloc(end - start + 1,sizeof(char));
    strncpy(slice, this->s + start, end - start);
    slice[end - start] = '\0';
    return slice;
}

void sstring_destroy(sstring *this) {
    // your code goes here
    free(this->s);
    this->len = 0;
    free(this);
}
