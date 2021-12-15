- If you desire to store formatted output in a buffer, a safe alternative to the standard Library *sprintf()* function. An even better choice is the *asprintf()* function.

  The *asprintf()* function sends formatted output into a perfectly-sized buffer, allocated within the function itself. Here is its *man* page format:

  int asprintf(char **ret, const char *format, ...);

  The first argument is the scary pointer-pointer thing, but it’s really the address of a pointer. When the function returns, the pointer contains the formatted output string; the final two arguments are the same as for the standard *printf()* function: a format string and variable argument list.

  Using this function is far safer than using the *sprintf()* function, which doesn’t check for buffer overflow. With the *asprintf()* function, the buffer size can be anything; all you must do is supply a *char* pointer to reference the string created. 

  

  

  Example:

  \#include <stdio.h>

  

  int main()

  {

  ​    char *buffer;

  ​    int r;

  

  ​    r = asprintf(&buffer,"The total is %d\n",5+8);

  

  ​    puts(buffer);

  ​    printf("%d characters generated\n",r);

  

  ​    return(0);

  }

  The *char* pointer `buffer` is declared at Line 5. Its address is passed to the *asprintf()* function at Line 8.

  Within the *asprintf()* function, the buffer is allocated, the size properly set to be filled with the formatted output. No overflow is possible.

  The buffer’s contents are output at Line 11. The *asprintf()* function’s return value, stored in variable `r`, is also output. This value is equal to the length of the string returned (the size of the buffer, minus one for the null character):