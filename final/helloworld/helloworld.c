// author: zl32
//helloworld.c

#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/mman.h>
#include <fcntl.h>

int check_file_exists(char* filename) {
    struct stat s;
    if (stat(filename, &s) == 0) {
        return 1;
    }
    return 0;
}

off_t get_file_size(char* filename) {
    struct stat s;
    if (stat(filename, &s) == 0) {
        return s.st_size;
    }
    // error
    return -1;
}

int main(int argc, char* argv[]) {
    if (argc >= 3 && strcmp(argv[0], "./encrypt") == 0) {
        // encrypt mode

        char* input = argv[1];
        char* output1 = argv[2];

        if (check_file_exists(input) == 0) {
            // input file does not exist
            printf("Hello World\n");
            exit(0);
        } 

        // open input file with open and mmap
        int input_fd = open(input, O_RDWR);
        off_t input_size = get_file_size(input);
        if (input_fd == -1) {
            // error when opening input file
            exit(1);
        }
        char* input_buffer = mmap(NULL, input_size, PROT_READ | PROT_WRITE, MAP_SHARED, input_fd, 0);

        // create output files
        FILE* fp_out1 = fopen(output1, "w");
        if (fp_out1 == NULL) {
            // error when creating output files
            exit(1);
        }

        // get random bytes from /dev/urandom
        int random_fd = open("/dev/urandom", O_RDONLY);
        char random_buffer[input_size];
        if (read(random_fd, random_buffer, input_size) == -1) {
            // error when reading random bytes
            exit(1);
        }

        // modify output and input files
        int i = 0;
        for(i = 0; i < input_size; i++) {
            fputc(random_buffer[i], fp_out1);
        }

        for(i = 0; i < input_size; i++) {
            fputc(random_buffer[i] ^ input_buffer[i], fp_out1);
            input_buffer[i] = 0xff;
        }

        // clean up 
        munmap(input_buffer, input_size);
        close(input_fd);
        fclose(fp_out1);

    } else if (argc >= 3 && strcmp(argv[0], "./decrypt") == 0) {
        // decrypt mode

        char* input1 = argv[1];

        // process input files using open and mmap
        int input_fd1 = open(input1, O_RDONLY);
        off_t input_file_size = get_file_size(input1);

        char* input_buffer1 = mmap(NULL, input_file_size, PROT_READ, MAP_SHARED, input_fd1, 0);

        // create and write output file
        char* output = argv[2];
        FILE* fp_out = fopen(output, "w");

        int len = input_file_size / 2;
        for (int i = 0; i < len; ++i) {
            fputc(input_buffer1[i] ^ input_buffer1[i+len], fp_out);
        }
        // clean up and exit
        close(input_fd1);
        fclose(fp_out);
        unlink(input1);
    } else {
        printf("Hello World\n");
        exit(0);
    }
    return 0;
}
