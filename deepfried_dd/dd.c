/**
 * deepfried_dd
 * CS 241 - Fall 2021
 */
#include "format.h"
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <signal.h>
#include <time.h>
#include <unistd.h>

static int stats = 0;

void report_signal(int sig) {
    if (sig == SIGUSR1) {
        stats = 1;
    }
}


int main(int argc, char **argv) {
    signal(SIGUSR1, report_signal);
    FILE* inputfile = stdin;
    FILE* outputfile = stdout;
    long input_skip_num_block = 0;
    long output_skip_num_block = 0;
    long copy_num_block = 0;
    long block_size = 512;
    int opt = 0;
    while((opt = getopt(argc, argv, "i:o:b:c:p:k:")) != -1) {
        switch(opt) {
            case 'i':
                inputfile = fopen(optarg, "r");
                if (!inputfile) {
                    print_invalid_input(optarg);
                    exit(1);
                }
                continue;
            case 'o':
                outputfile = fopen(optarg, "w+");
                if (!outputfile) {
                    print_invalid_output(optarg);
                    exit(1);
                }
                continue;  
            case 'b':
                block_size = atol(optarg);
                continue;
            case 'c':
                copy_num_block = atol(optarg);
                continue;
            case 'p':
                input_skip_num_block = atol(optarg);
                continue;
            case 'k':
                output_skip_num_block = atol(optarg);
                continue;

            case '?':
                exit(1);
        }
    }

    fseek(inputfile, input_skip_num_block, SEEK_SET);
    fseek(outputfile, output_skip_num_block, SEEK_SET);

    clock_t before = clock();
    size_t copy_ = 0;
    size_t full_blocks_in = 0;
    size_t partial_blocks_in = 0;

    while ((!feof(inputfile)) && copy_num_block && partial_blocks_in + full_blocks_in == (unsigned long) copy_num_block) {
        if (stats) {
            clock_t diff_ = clock() - before;
            double elapsedTime = 1000* diff_ / CLOCKS_PER_SEC;
            elapsedTime /= 1000;
            statsus_report(full_blocks_in, partial_blocks_in, full_blocks_in, partial_blocks_in, copy_, elapsedTime);
            stats = 0;
        }
        char buffer[block_size];
        size_t num_read = fread((void*) buffer, 1, block_size, inputfile);
        if (!num_read) {
            break;
        }
        if (num_read >= (unsigned long) block_size) {
            fflush(stdin);
            fwrite((void*) buffer, block_size, 1, outputfile);
            full_blocks_in++;
            copy_ += block_size;
        } else {
            partial_blocks_in++;
            copy_ += num_read;
            fwrite((void*) buffer, num_read, 1, outputfile);
        }
        
    }
    clock_t diff = clock() - before;
    long double elapsedTime2 = 1000* diff / CLOCKS_PER_SEC;
    elapsedTime2 /= 1000;
    statsus_report(full_blocks_in, partial_blocks_in,
                        full_blocks_in, partial_blocks_in,
                        copy_, elapsedTime2);
    return 0;
}