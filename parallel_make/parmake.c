/**
 * parallel_make
 * CS 241 - Fall 2021
 */

#include "format.h"
#include "graph.h"
#include "parmake.h"
#include "parser.h"
#include "queue.h"
#include "vector.h"
#include "dictionary.h"
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <time.h>
#include <fcntl.h>
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>

pthread_mutex_t mux = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t cond = PTHREAD_COND_INITIALIZER;

graph* g = NULL;
queue* q = NULL;
dictionary* d = NULL;
size_t thread_count = 0;

void quu_push(char* ptr);
int setup(void* ptr);
int detect_cycle(void* ptr);
void* part2(void* ptr);



int parmake(char *makefile, size_t num_threads, char **targets) {
    // good luck!
    //set up
    d = string_to_int_dictionary_create();
    g = parser_parse_makefile(makefile, targets);
    q = queue_create(-1);
    pthread_t pids[num_threads];
    vector* goals = graph_neighbors(g, "");
    //detect cycle
    for (size_t i = 0; i < vector_size(goals); i++) {
        char* curr = vector_get(goals, i);
        if (setup((void*)curr) == 1) {
            print_cycle_failure(curr);
            rule_t* curr_rule = (rule_t*) graph_get_vertex_value(g, (void*)curr);
            curr_rule->state = -1;
            vector_erase(goals, i);
            i--;
        }
    }

    rule_t *root = graph_get_vertex_value(g, "");
    root->state = vector_size(goals);
    if (vector_empty(goals)) {return 0;}
    //push goals to the queue
    int zero = 0;
    vector* vertices = graph_vertices(g);
    VECTOR_FOR_EACH(vertices, curr, {dictionary_set(d, curr, &zero);});
    vector_destroy(vertices);
    VECTOR_FOR_EACH(goals, vtx, {quu_push(vtx);});
    //multi thread set up and calculation
    dictionary_destroy(d);
    for (size_t i = 0; i < num_threads; i++) {
        pthread_create(pids + i, NULL, part2, NULL);
    }
    pthread_mutex_lock(&mux);
    while (thread_count != vector_size(goals)) {
        pthread_cond_wait(&cond, &mux);
    }
    pthread_mutex_unlock(&mux);
    for (size_t i = 0; i < num_threads + 1; i++) {
        queue_push(q, NULL);
    }
    for (size_t i = 0; i < num_threads; i++) {
        pthread_join(pids[i], NULL);
    }
    vector_destroy(goals);
    graph_destroy(g);   
    queue_destroy(q);
    pthread_cond_destroy(&cond);
    pthread_mutex_destroy(&mux);
    return 0;
}



void quu_push(char* ptr) {
    if (*(int*)dictionary_get(d, ptr) == 1) {
        return;
    }
    int one = 1;
    dictionary_set(d, ptr, &one);

    vector* neibor = graph_neighbors(g, ptr);
    VECTOR_FOR_EACH(neibor, 
        varname, 
        {quu_push(varname);});

    rule_t* val = (rule_t *)graph_get_vertex_value(g, ptr);
    val->state = vector_size(neibor);

    if (vector_empty(neibor)) {
        queue_push(q, ptr);
    }
    vector_destroy(neibor);
}


int setup(void* ptr) {
    vector* vecname = graph_vertices(g);

    int zero = 0;
    VECTOR_FOR_EACH(vecname, 
        varname, 
        {dictionary_set(d, varname, &zero);});
    vector_destroy(vecname);
    return detect_cycle(ptr);
}

int detect_cycle(void* ptr) {
    if (!dictionary_contains(d, ptr)) {
        return 0;
    }

    if (*(int*)dictionary_get(d, ptr) == 1) {
        return 1;
    }

    if(*(int*)dictionary_get(d, ptr) == 2) {
        return 2;
    }

    vector* neibor = graph_neighbors(g, ptr);
    size_t total_neibor = vector_size(neibor);


    for (size_t i = 0; i < total_neibor; i++) {
        void* curr = vector_get(neibor, i);
        if (detect_cycle(curr) == 1) {
            int one = 1;
            dictionary_set(d, ptr, &one);
            vector_destroy(neibor);
            return 1;
        }
    }

    int two = 2;
    dictionary_set(d, ptr, &two);
    vector_destroy(neibor);
    return 0;
}



void* part2(void* ptr) {
    while (true) {
        char* curr = (char*)queue_pull(q);
        if (!curr) {
            break;
        }

        int flag = 1;
        int check_stat = 0;
        struct stat stat_;

        rule_t* val = (rule_t*)graph_get_vertex_value(g, curr);
        if (stat(val->target, &stat_) == -1) {
            check_stat = 1;
        }
        
        if (!check_stat) {
            pthread_mutex_lock(&mux);
            vector* neibor = graph_neighbors(g, curr);
            pthread_mutex_unlock(&mux);
            VECTOR_FOR_EACH(neibor, 
                varname, 
                {
                    rule_t* tempr = (rule_t*)graph_get_vertex_value(g, varname);
                    struct stat stat2;
                    if (stat(tempr->target, &stat2) == -1 || stat2.st_mtime > stat_.st_mtime) {
                        check_stat = 1;
                        break;
                    }
                });
            vector_destroy(neibor);
        }

        if (check_stat) {
            VECTOR_FOR_EACH(val->commands, 
                varname, 
                {
                    if (system(varname)) {
                        flag = 0;
                        break;
                    }
                });
        }

        pthread_mutex_lock(&mux);
        vector* anti_neibor = graph_antineighbors(g, curr);
        VECTOR_FOR_EACH(anti_neibor, 
            varname, 
            {
                if (flag) {
                    rule_t* val_ = graph_get_vertex_value(g, varname);
                    val_->state --;
                    if (!val_->state) {
                        queue_push(q, varname);
                    }
                }

                if (!strcmp(varname, "")) {
                    thread_count++;
                    pthread_cond_signal(&cond);
                }
        });

        vector_destroy(anti_neibor);
        pthread_mutex_unlock(&mux);
    }

    return ptr;
}