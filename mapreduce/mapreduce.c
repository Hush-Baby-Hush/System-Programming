/**
 * mapreduce
 * CS 241 - Fall 2021
 */
 // reference: https://github.com/Haoyuliu-ooyu/UIUC-System-Programming/blob/master/mapreduce/mapreduce.c
#include "utils.h"
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/stat.h>
#include <fcntl.h>


int main(int argc, char **argv) {
    // Create an input pipe for each mapper.

    // Create one input pipe for the reducer.

    // Open the output file.

    // Start a splitter process for each mapper.

    // Start all the mapper processes.

    // Start the reducer process.

    // Wait for the reducer to finish.

    // Print nonzero subprocess exit codes.

    // Count the number of lines in the output file.
    if (argc != 6) {
        print_usage();
        return 1;
    }

    //char* input_file = argv[1];
    //char* output_file = argv[2];
    int mapper_count;
    if (sscanf(argv[5], "%d", &mapper_count != 1) || mapper_count < 1) {
      print_usage();
      return 1;
    }

    //char* mapper = argv[3];
    //char* reducer = argv[4];
    int* fd[mapper_count];
    for (int i = 0; i< mapper_count; i++) {
        fd[i] = calloc(2, sizeof(int));
        pipe(fd[i]);
    }
    int fd_r[2];
    pipe(fd_r);
    char* output_file = argv[2];
    int file = open(output_file, O_CREAT | O_WRONLY | O_TRUNC, S_IWUSR | S_IRUSR);

    pid_t child_split[mapper_count];
    char* input_file = argv[1];
    for (int i = 0; i< mapper_count; i++){
        child_split[i] = fork();
        if (!child_split[i]) { 
            close(fd[i][0]);
            char temp[16];
            sprintf(temp, "%d", i);
            dup2(fd[i][1], 1);
            execl("./splitter", "./splitter", input_file, argv[5], temp, NULL);
            exit(1);
        }
    }
    pid_t child_mapping[mapper_count];
    char* mapper = argv[3];
    for (int i = 0; i < mapper_count; i++) {
        close(fd[i][1]);
        child_mapping[i] = fork();
        if (!child_mapping[i]) {
            close(fd_r[0]);
            dup2(fd[i][0], 0);
            dup2(fd_r[1], 1);
            execl(mapper, mapper, NULL);
            exit(1);
        }
    }
    close(fd_r[1]);
    //pid_t child = ;
    char* reducer = argv[4];
    if (!fork()) {
        dup2(fd_r[0], 0);
        dup2(file, 1);
        execl(reducer, reducer, NULL);
        exit(1);
    }
    close(fd_r[0]);
    close(file);
    
    for (int i = 0; i < mapper_count; i++) {
        int st;
        waitpid(child_split[i], &st, 0);
    } 
    
    for (int i = 0; i < mapper_count; i++) {
        close(fd[i][0]);
        int st;
        waitpid(child_mapping[i], &st, 0);
    }
    int st;
    waitpid(child, &st, 0);
    if (st) {
        print_nonzero_exit_status(reducer, st);
    }

    print_num_lines(output_file);
    
    for (int i = 0; i< mapper_count; i++) {
        free(fd[i]);
    }
    return 0;
}
