// author: zl32
//helloworld.c

#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <fcntl.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/mman.h>


int exist_file(char* name) {
    struct stat s;
    if (stat(name, &s) == 0) {
        return 1;
    }
    return 0;
}

off_t get_file_size(char* name) {
    struct stat s;
    if (stat(name, &s) == 0) {
        return s.st_size;
    }
    return -1;
}

int main(int argc, char* argv[]) {
    if (argc >= 3 && strcmp(argv[0], "./encrypt") == 0) {

        char* input = argv[1];
        char* output = argv[2];

        if (exist_file(input) == 0) {
            printf("Hello World\n");
            exit(0);
        } 

        int infd = open(input, O_RDWR);
        off_t getfSize = get_file_size(input);
        if (infd == -1) {
            exit(1);
        }
        char* inbuff = mmap(NULL, getfSize, PROT_READ | PROT_WRITE, MAP_SHARED, infd, 0);

        FILE* file1 = fopen(output, "w");
        if (file1 == NULL) {
            exit(1);
        }

        int rfd = open("/dev/urandom", O_RDONLY);
        char rbuff[getfSize];
        if (read(rfd, rbuff, getfSize) == -1) {
            exit(1);
        }

        for(int i = 0; i < getfSize; i++) {
            fputc(rbuff[i], file1);
        }

        for(int i = 0; i < getfSize; i++) {
            fputc(rbuff[i] ^ inbuff[i], file1);
            inbuff[i] = 0xff;
        }

        munmap(inbuff, getfSize);
        close(infd);
        fclose(file1);

    } else if (argc >= 3 && strcmp(argv[0], "./decrypt") == 0) {
        char* input = argv[1];
        int infd1 = open(input, O_RDONLY);
        off_t getfSize = get_file_size(input);
        char* inbuff = mmap(NULL, getfSize, PROT_READ, MAP_SHARED, infd1, 0);

        char* output = argv[2];
        FILE* outfile = fopen(output, "w");

        int len = getfSize / 2;
        for (int i = 0; i < len; ++i) {
            fputc(inbuff[i] ^ inbuff[i+len], outfile);
        }

        close(infd1);
        fclose(outfile);
        unlink(input);
    } else {
        printf("Hello World\n");
        exit(0);
    }
    return 0;
}
