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

void push_to_queue(char* ptr);
int setup(void* ptr);
int detect_cycle(void* ptr);
void* part2(void* ptr);



int parmake(char *makefile, size_t num_threads, char **targets) {
    // good luck!

    q = queue_create(-1);
    d = string_to_int_dictionary_create();
    g = parser_parse_makefile(makefile, targets);
    if (num_threads < 1) {
        return 0;
    }

    vector* neibor = graph_neighbors(g, "");
    if (vector_empty(neibor)) {
        rule_t* ru = graph_get_vertex_value(g, "");
        ru->state = vector_size(neibor);
        return 0;
    }

    for (size_t i = 0; i < vector_size(neibor); i++) {
        char* curr = vector_get(neibor, i);
        if (setup((void*)curr) == 1) {
            print_cycle_failure(curr);
            rule_t* rul = (rule_t*) graph_get_vertex_value(g, (void*)curr);
            rul->state = -1;
            vector_erase(neibor, i);
            i--;
        }
    }

    rule_t* ru = graph_get_vertex_value(g, "");
    ru->state = vector_size(neibor);

    int zero = 0;
    vector* vertices = graph_vertices(g);
    VECTOR_FOR_EACH(vertices, 
        varname, 
        {dictionary_set(d, varname, &zero);});

    vector_destroy(vertices);

    VECTOR_FOR_EACH(neibor, 
        varname, 
        {quu_push(varname);});

    dictionary_destroy(d);

    pthread_t threads[num_threads];
    for (size_t i = 0; i < num_threads; i++) {
        pthread_create(threads + i, NULL, part2, NULL);
    }

    pthread_mutex_lock(&mux);
    while (thread_count != vector_size(neibor)) {
        pthread_cond_wait(&cond, &mux);
    }
    pthread_mutex_unlock(&mux);
    
    for (size_t i = 0; i < num_threads + 1; i++) {
        queue_push(q, NULL);
    }
    for (size_t i = 0; i < num_threads; i++) {
        pthread_join(threads[i], NULL);
    }

    queue_destroy(q);
    vector_destroy(neibor);
    graph_destroy(g);   
    pthread_cond_destroy(&cond);
    pthread_mutex_destroy(&mux);
    return 0;
}



void push_to_queue(char *target) {
    if (*(int*)dictionary_get(d, target) == 1) return;
    int one = 1;
    dictionary_set(d, target, &one);
    vector* dependencies = graph_neighbors(g, target);
    //push for each item in dependencies
    VECTOR_FOR_EACH(dependencies, vt, {push_to_queue(vt);});
    if (vector_empty(dependencies)) queue_push(q, target);
    rule_t *rule = (rule_t *)graph_get_vertex_value(g, target);
    rule->state = vector_size(dependencies);
    vector_destroy(dependencies);
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