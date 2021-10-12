# Homework 0

HW0 questions are below. Copy your answers into a text document (which the course staff will grade later) because you'll need to submit them later in the course. More information will be in the first lab.

## Chapter 1

In which our intrepid hero battles standard out, standard error, file descriptors and writing to files.

### Hello, World! (system call style)
1. Write a program that uses `write()` to print out "Hi! My name is `<Your Name>`".

   ```c
   #include <unistd.h>
   int main() {
       int len = strlen("Hi! My name is Kimmy Liu");
       write(1, "Hi! My name is Kimmy Liu\n", len+2);
       return 0;
   }
   ```

   

### Hello, Standard Error Stream!
2. Write a function to print out a triangle of height `n` to standard error.
   - Your function should have the signature `void write_triangle(int n)` and should use `write()`.
   - The triangle should look like this, for n = 3:
   ```C
   *
   **
   ***
   ```
   
   ```c
   #include <unistd.h>
   
   void write_triangle(int n){
   	if(n<=0){
   		exit(1);
   	}
   	int count, line;
   	for(line = 1; line<n+1; line++){
   		
   		for(count = 0; count < line; count++){
   			write(STDERR_FILENO, "*", 1);
   		}
   		write(STDERR_FILENO, "\n", 1);
   		
   	}
   }
   
   int main() {
   	int n = 3;
   	write_triangle(n);
   	return 0;
   }
   ```
   
   



### Writing to files

3. Take your program from "Hello, World!" modify it write to a file called `hello_world.txt`.
   - Make sure to to use correct flags and a correct mode for `open()` (`man 2 open` is your friend).
   
   ```c
   #include <sys/types.h>
   #include <sys/stat.h>
   #include <fcntl.h>
   #include <unistd.h>
   int main() {
   mode_t mode = S_IRUSR | S_IWUSR;
   int fildes = open("hello_world.txt", O_CREAT | O_TRUNC | O_RDWR, mode);
   int len = strlen("Hi! My name is Kimmy Liu");
   write(fildes, "Hi! My name is Kimmy Liu\n", len+2);
   close(fildes);
   return 0;}
   ```
   
   
### Not everything is a system call
4. Take your program from "Writing to files" and replace `write()` with `printf()`.
   - Make sure to print to the file instead of standard out!
   
   ```
   #include <string.h>
   #include <sys/types.h>
   #include <sys/stat.h>
   #include <fcntl.h>
   #include <unistd.h>
   #include <stdio.h>
   
   int main() {
   	close(1);
   	mode_t mode = S_IRUSR | S_IWUSR;
   	int fildes = open("hello_world.txt", O_CREAT | O_TRUNC | O_RDWR, mode);
   	printf("%s", "Hi! My name is Kimmy Liu\n");
   	close(fildes);
   	return 0;
   }
   ```
   
   
   
5. What are some differences between `write()` and `printf()`?

   ```
   write() is designed to only write a sequence of bytes. 
   
   printf() can lets us write data in many different formats. 
   
   So, we can understand printf() as a function that convert data into a formatted sequence of bytes and that calls `write()` to write those bytes onto the output. 
   
   Write is a system call and printf is a library call. 
   
   ---learn from stackoverflow
   ```

   

## Chapter 2

Sizing up C types and their limits, `int` and `char` arrays, and incrementing pointers

### Not all bytes are 8 bits?
1. How many bits are there in a byte?

   ```
   8
   ```

   

2. How many bytes are there in a `char`?

   ```
   1
   ```

   

3. How many bytes the following are on your machine?
   - `int`, `double`, `float`, `long`, and `long long`

   ```
   4, 8, 4, 4, 8
   ```

   

### Follow the int pointer
4. On a machine with 8 byte integers:
```C
int main(){
    int data[8];
} 
```
If the address of data is `0x7fbd9d40`, then what is the address of `data+2`?

```
0x7fbd9d50
```



5. What is `data[3]` equivalent to in C?
   - Hint: what does C convert `data[3]` to before dereferencing the address?
   
   ```
   data+3
   ```
   
   

### `sizeof` character arrays, incrementing pointers

Remember, the type of a string constant `"abc"` is an array.

6. Why does this segfault?
```C
char *ptr = "hello";
*ptr = 'J';
```
​	

```
Because string is constant and immutable. It is stored in the system memory and can't change. 
```



7. What does `sizeof("Hello\0World")` return?

```
12
```



7. What does `strlen("Hello\0World")` return?

```
5
```



7. Give an example of X such that `sizeof(X)` is 3.

```
X = "hi"
```



7. Give an example of Y such that `sizeof(Y)` might be 4 or 8 depending on the machine.

```
int Y = 7

int size might be 4 or 8 depending on the machine
```



## Chapter 3

Program arguments, environment variables, and working with character arrays (strings)

### Program arguments, `argc`, `argv`
1. What are two ways to find the length of `argv`?

   ```
   a. argc is the length of argv
   
   b. loop over argv[index] until point to null. 
   ```

   

2. What does `argv[0]` represent?

   ```
   execution name of program, that how we start the program. 
   
   ./program_name
   ```

   

### Environment Variables
3. Where are the pointers to environment variables stored (on the stack, the heap, somewhere else)?

   ```
   They are stored above the stack, at top of the process memory layout.
   
   They are stored in the process' own memory not in files. 
   ```

   

### String searching (strings are just char arrays)
4. On a machine where pointers are 8 bytes, and with the following code:
```C
char *ptr = "Hello";
char array[] = "Hello";
```
What are the values of `sizeof(ptr)` and `sizeof(array)`? Why?

```
sizeof(ptr) = 8, because pointers are 8 bytes. 
sizeof(array) = 6, because array represent 5 bytes used to store hello and a \0 at end. 
```



### Lifetime of automatic variables

5. What data structure manages the lifetime of automatic variables?

```
Stack
```



## Chapter 4

Heap and stack memory, and working with structs

### Memory allocation using `malloc`, the heap, and time
1. If I want to use data after the lifetime of the function it was created in ends, where should I put it? How do I put it there?

```
Heap; using malloc, realloc, and calloc
```

2. What are the differences between heap and stack memory?	

```
Stack is used for static memory allocation while heap is used for dynamic memory allocation.
Variables allocated on the stack are stored directly to the memory and access to this memory is very fast. 
Stacks get meaningless by stack pointer after used. 
Heap needs to be freed while stack not. 
```

3. Are there other kinds of memory in a process?

   ```
   Data Segment, Text Segment. 
   ```

   

4. Fill in the blank: "In a good C program, for every malloc, there is a ___".

   ```
   free
   ```

   

### Heap allocation gotchas
5. What is one reason `malloc` can fail?

   ```
   There is no enough space 
   ```

   

6. What are some differences between `time()` and `ctime()`?

   ```
   Time() is of type time_t. The value returned generally represents the number of seconds since 00:00 hours, Jan 1, 1970 UTC (i.e., the current unix timestamp).
   
   Ctime() convert time_t value to string. Interprets the value pointed by timer as a calendar time and converts it to a C-string containing a human-readable version of the corresponding time and date, in terms of local time. 
   
   --learn from https://www.cplusplus.com
   ```

   

7. What is wrong with this code snippet?

```C
free(ptr);
free(ptr);
```
```
double free
```



8. What is wrong with this code snippet?
```C
free(ptr);
printf("%s\n", ptr);
```
```
reuse of already freed pointers
```



9. How can one avoid the previous two mistakes? 

```
Set freed pointers to NULL to avoid dangling pointers.
```



### `struct`, `typedef`s, and a linked list
10. Create a `struct` that represents a `Person`. Then make a `typedef`, so that `struct Person` can be replaced with a single word. A person should contain the following information: their name (a string), their age (an integer), and a list of their friends (stored as a pointer to an array of pointers to `Person`s).

```
#include <stdio.h>

struct Person{
char* name;
int age;
struct Person* friends[10];
}

typedef struct Person person;
```



11. Now, make two persons on the heap, "Agent Smith" and "Sonny Moore", who are 128 and 256 years old respectively and are friends with each other.

```
#include <stdio.h>

struct Person{
char* name;
int age;
struct Person* friends[10];
}

typedef struct Person person;

int main() {
person* p1 = (person*) malloc(sizeof(person));
person* p2 = (person*) malloc(sizeof(person));
p1 -> name = "Agent Smith";
p1 -> age = 128;
p2 -> name = "Sonny Moore";
p2 -> age = 256;
p1 -> friends[0] = p2;
p2 -> friends[0] = p1;
free(p1->name);
free(p2->name);
memset(p2, 0 , sizeof(person));
memset(p1, 0 , sizeof(person));
free(p2);
free(p1);
return 0;
}

```



### Duplicating strings, memory allocation and deallocation of structures
Create functions to create and destroy a Person (Person's and their names should live on the heap).
12. `create()` should take a name and age. The name should be copied onto the heap. Use malloc to reserve sufficient memory for everyone having up to ten friends. Be sure initialize all fields (why?).

```
person* create(char* name, int age){
  person* person_ = (person*) malloc(sizeof(person));
  person_->name = strdup(name);
  person_->age = age;
  for(int count = 10; count; count--){
    person_->friends[count] = NULL;
  }
  return person_;
}
```



13. `destroy()` should free up not only the memory of the person struct, but also free all of its attributes that are stored on the heap. Destroying one person should not destroy any others.

```
void destroy(person*  person_){
  free(person_->name);
  memset(person_, 0 , sizeof(person));
  free(person_);
}
```



## Chapter 5 

Text input and output and parsing using `getchar`, `gets`, and `getline`.

### Reading characters, trouble with gets
1. What functions can be used for getting characters from `stdin` and writing them to `stdout`?

   ```
   gets();puts()
   ```

   

2. Name one issue with `gets()`.

   ```
   It needs to have a buffer declared and overflow of input will change other variables and you can't tell if overflow or not.
   ```

   

### Introducing `sscanf` and friends
3. Write code that parses the string "Hello 5 World" and initializes 3 variables to "Hello", 5, and "World".

   ```
   #include <stdio.h>
   
   int main() {
       char * data = "Hello 5 World";
       char buffer1[20];
       int score = -42;
       char buffer2[20];
       sscanf(data, "%s %d %s", buffer1, & score, buffer2);
       return 0;
   }
   ```

   

### `getline` is useful
4. What does one need to define before including `getline()`?

   ```
   #define _GNU_SOURCE
   ```

   

5. Write a C program to print out the content of a file line-by-line using `getline()`.

   ```
   #define _GNU_SOURCE
   #include <stdio.h>
   #include <stdlib.h>
   
   int main(int argc, char *argv[]) {
       FILE * fp = fopen(argv[1], "r"); 
       char *buffer = NULL;
       size_t capacity = 0;
       ssize_t result = getline(&buffer, &capacity, fp);
       while (result!=-1){
           printf("%s\n", buffer);
           result = getline(&buffer, &capacity, fp);
       }
   }
   ```

   

## C Development

These are general tips for compiling and developing using a compiler and git. Some web searches will be useful here

1. What compiler flag is used to generate a debug build?

   ```
   -g
   ```

   

2. You modify the Makefile to generate debug builds and type `make` again. Explain why this is insufficient to generate a new build.

   ```
   Need to "make clean" first to clear previous .o files. 
   ```

   

3. Are tabs or spaces used to indent the commands after the rule in a Makefile?

   ```
   Tabs are used to do that.
   ```

   

4. What does `git commit` do? What's a `sha` in the context of git?

   ```
   The git commit command captures a snapshot of the project's currently staged changes.
   
   "SHA" stands for Simple Hashing Algorithm. With git revert, the SHA is used to specify which commit contains the set of changes you want to undo.
   ```

   

5. What does `git log` show you?

   ```
   find a commit you want to undo.
   ```

   

6. What does `git status` tell you and how would the contents of `.gitignore` change its output?

   ```
   The git status command displays the state of the working directory and the staging area. It lets you see which changes have been staged, which haven't, and which files aren't being tracked by Git. Status output does not show you any information regarding the committed project history. 
   
   Git lets you completely ignore files by placing paths in a special file called .gitignore. Any files that you'd like to ignore should be included on a separate line, and the * symbol can be used as a wildcard.
   
   ---from https://www.atlassian.com/git/tutorials/inspecting-a-repository
   
   
   ```

   

7. What does `git push` do? Why is it not just sufficient to commit with `git commit -m 'fixed all bugs' `?

   ```
   The git push command is used to upload local repository content to a remote repository.
   
   It is not sufficient because sometimes commits aren't ready to be pushed and also, you may have multiple remotes. Pushing must be explicit from git's perspective due to the semantics of git being a distributed VCS. -- learn from stackoverflow
   ```

   

8. What does a non-fast-forward error `git push` reject mean? What is the most common way of dealing with this?

   ```
   With this error message Gerrit rejects a push if the remote branch can't be fast forwarded onto the pushed commit. 
   
   Usually this is caused by another user pushing to the same branch. You can remedy this by fetching and merging the remote branch, or using pull to perform both at once.
   
   --learn from stackoverflow.
   ```

   

## Optional (Just for fun)
- Convert your a song lyrics into System Programming and C code and share on Ed.
- Find, in your opinion, the best and worst C code on the web and post the link to Ed.
- Write a short C program with a deliberate subtle C bug and post it on Ed to see if others can spot your bug.
- Do you have any cool/disastrous system programming bugs you've heard about? Feel free to share with your peers and the course staff on Ed.
