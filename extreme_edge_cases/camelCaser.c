/**
 * extreme_edge_cases
 * CS 241 - Fall 2021
 */
#include "camelCaser.h"
#include <stdlib.h>
#include <ctype.h>
#include <string.h>


char** empCharPtrArr(const char* input_str) {
    int i = 0;
    int punc = 0;
    while (input_str[i])
    {
        if (ispunct(input_str[i]))
        {
            punc++;
        }
        i++;
    }

    char ** outerPtr = (char **) calloc(punc+1,sizeof(char*));
    outerPtr[punc] = NULL;
    return outerPtr;
    
}

  
char** CharPtrArr_space(const char* input_str, char** outerPtr) {

    int i = 0;
    int j = 0;
    int inner = 0;
    while (input_str[i])
    {
        if (ispunct(input_str[i]))
        {

            outerPtr[inner] = (char *) calloc(j+1, sizeof(char));
            strncpy(outerPtr[inner],&input_str[i-j], j);
            outerPtr[inner][j] = '\0';
            j = 0;
            inner++;

        }else
        {
            j++;
        }
        i++;
    }

    return outerPtr;

}

char** CharPtrArr(char** outerPtr) {

    int inner = 0;
    while (outerPtr[inner]!=NULL)
    {
        char* nospace = calloc(strlen(outerPtr[inner])+1, sizeof(char));
        int i = 0;
        int len = 0;
        int word = 0;
        int flag = 0;
        while (outerPtr[inner][i]!='\0')
        {
            if (!isspace(outerPtr[inner][i]))
            {
                if (isalpha(outerPtr[inner][i]))
                {
                    if (!word)
                    {
                        word = 1;
                        nospace[len] = tolower(outerPtr[inner][i]);
                        len++;
                        i++;
                        continue;
                    }else if (flag)
                    {
                        nospace[len] = toupper(outerPtr[inner][i]);
                        len++;
                        flag = 0;
                    }else
                    {
                        nospace[len] = tolower(outerPtr[inner][i]);
                        len++;
                    }
                    
                }else
                {
                    word++;
                    nospace[len] = outerPtr[inner][i];
                    len++;
                }

            }else
            {
                if (!word)
                {
                    i++;
                    flag = 0;
                    continue;
                }else
                {
                    i++;
                    flag = 1;
                    continue;
                }
                
            }

            i++;
            
        }

        nospace[len] = '\0';
        nospace = realloc(nospace, len+1);
        free(outerPtr[inner]);
        outerPtr[inner] = nospace;
        inner++;
        
    }



    return outerPtr;

}



char **camel_caser(const char *input_str) {
    // TODO: Implement me!
    if (!input_str)
    {
        return NULL;
    }
    char** camel_case = empCharPtrArr(input_str);
    camel_case = CharPtrArr_space(input_str, camel_case);
    camel_case =  CharPtrArr(camel_case);
    return camel_case;

}

void destroy(char **result) {
    // TODO: Implement me!
    if (!result)
    {
        return;
    }
    int i = 0;
    while (result[i]!=NULL) {
        free(result[i]);
        i++;
    }
    free(result);
    return;
}
