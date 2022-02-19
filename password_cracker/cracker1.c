/**
 * password_cracker
 * CS 241 - Fall 2021
 */
#include "cracker1.h"
#include "format.h"
#include "utils.h"
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include "./includes/queue.h"
#include <pthread.h>
#include <string.h>
#include <crypt.h>


static queue* q_task;
pthread_mutex_t mux = PTHREAD_MUTEX_INITIALIZER;
static int total_num;
static int success;


void* crack(void* index_) {
    // pthread_mutex_lock(&mux);
    char username[9];
    char password[14];
    char known[9];
    size_t index = (size_t) index_;
    struct crypt_data crypt;
    crypt.initialized = 0;

    char* task = NULL;
    while (true) {
        task = queue_pull(q_task);
        if (!task) {
            break;
        }

        sscanf(task, "%s %s %s", username, password, known);
        v1_print_thread_start(index, username);
        int len_ = getPrefixLength(known);
        char* ptr = len_ + known;
        setStringPosition(ptr, 0);
        double time = getThreadCPUTime();

        int count = 0;
        int fail = 1;
        char* hash = NULL;
        while (1) {
            hash = crypt_r(known, "xx", &crypt);
            if (!strcmp(hash, password)) {
                pthread_mutex_lock(&mux);
                success++;
                pthread_mutex_unlock(&mux);
                fail = 0;
                break;
            }
            if (!incrementString(ptr)) { break; }
            count++;

        }
        double finaltime = getThreadCPUTime() - time;
        v1_print_thread_result(index, username, known, count, finaltime, fail);
        free(task);
        task = NULL;
    }
    // pthread_mutex_unlock(&mux);
    return NULL;
}


int start(size_t thread_count) {
    // TODO your code here, make sure to use thread_count!
    // Remember to ONLY crack passwords in other threads

    q_task = queue_create(0);
    pthread_t arr[thread_count];
    size_t len = 0;
    char* line = NULL;

    while(getline(&line, &len, stdin) != -1) {
        if (strlen(line)>0 && line[strlen(line) - 1] == '\n') {
            line[strlen(line) - 1] = '\0';
        }   
        queue_push(q_task, strdup(line));
        total_num++;
    }

    free(line);
    
    for(size_t i = 0; i < thread_count; i++) {
        queue_push(q_task, NULL);
    }

    for(size_t i = 0; i < thread_count; i++) {
        pthread_create(arr + i, NULL, crack, (void*) i + 1);
    }

    for(size_t i = 0; i < thread_count; i++) {
        pthread_join(arr[i], NULL);
    }

    v1_print_summary(success, total_num - success);

    pthread_mutex_destroy(&mux);
    queue_destroy(q_task);

    return 0; // DO NOT change the return code since AG uses it to check if your
              // program exited normally
}
