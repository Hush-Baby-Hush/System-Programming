/**
 * password_cracker
 * CS 241 - Fall 2021
 */
#include "cracker1.h"
#include "format.h"
#include "utils.h"
#include "./includes/queue.h"
#include <pthread.h>
#include <string.h>
#include <crypt.h>
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>

pthread_mutex_t lock = PTHREAD_MUTEX_INITIALIZER;
// task queue
static queue* tasks;
// total number of tasks
static size_t num_tasks;
// number of cracked passwords
static int recovered_num;


void* crack_password(void* thread_num) {
    size_t index = (size_t) thread_num;
    char username[10];
    char hash[16];
    char known[16];
    struct crypt_data cdata;
    cdata.initialized = 0;
    char* task = NULL;
    while (true) {
        task = queue_pull(tasks);
        if (!task) {
            break;
        }
        // get corresponding parts
        sscanf(task, "%s %s %s", username, hash, known);
        // print starting info
        v1_print_thread_start(index, username);
        int prefix_length = getPrefixLength(known);
        // set to first unknown
        setStringPosition(prefix_length + known, 0);
        int hash_count = 0;
        double start_time = getThreadCPUTime();
        char* current_hash = NULL;
        int fail = 1;
        // finding solution
        while (1) {
            current_hash = crypt_r(known, "xx", &cdata);
            hash_count++;
            // found solution
            if (strcmp(current_hash, hash) == 0) {
                pthread_mutex_lock(&lock);
                recovered_num++;
                pthread_mutex_unlock(&lock);
                fail = 0;
                break;
            }
            // increment fail. cannot recover
            int result = incrementString(prefix_length + known);
            if (result == 0) {
                break;
            }
        }
        double time = getThreadCPUTime() - start_time;
        // print info and free stuff
        v1_print_thread_result(index, username, known, hash_count, time, fail);
        free(task);
        task = NULL;
    }
    return NULL;
}

int start(size_t thread_count) {
    // TODO your code here, make sure to use thread_count!
    // Remember to ONLY crack passwords in other threads

    tasks = queue_create(0);
    pthread_t arr[thread_count];
    size_t len = 0;
    char* line = NULL;

    while(getline(&line, &len, stdin) != -1) {
        if (strlen(line)>0 && line[strlen(line) - 1] == '\n') {
            line[strlen(line) - 1] = '\0';
        }   
        queue_push(tasks, strdup(line));
        num_tasks++;
    }

    free(line);
    
    for(size_t i = 0; i < thread_count; i++) {
        queue_push(tasks, NULL);
    }

    for(size_t i = 0; i < thread_count; i++) {
        pthread_create(arr + i, NULL, crack_password, (void*) i + 1);
    }

    for(size_t i = 0; i < thread_count; i++) {
        pthread_join(arr[i], NULL);
    }

    v1_print_summary(recovered_num, num_tasks - recovered_num);

    pthread_mutex_destroy(&lock );
    queue_destroy(tasks);

    return 0; // DO NOT change the return code since AG uses it to check if your
              // program exited normally
}

