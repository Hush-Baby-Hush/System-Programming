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
void* par_run(void* ptr);



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
        pthread_create(threads + i, NULL, par_run, NULL);
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



void* par_run(void* ptr) {
    while (1) {
        char* target = (char*)queue_pull(q);
        if (!target) {break;}
        rule_t* rule = (rule_t*)graph_get_vertex_value(g, target);
        int done = 1;
        int flag = 0;
        struct stat stat_inf;
        if (stat(rule->target, &stat_inf) == -1) {flag = 1;}
        if (flag == 0) {
            pthread_mutex_lock(&mux);
            vector* dependencies = graph_neighbors(g, target);
            pthread_mutex_unlock(&mux);
            VECTOR_FOR_EACH(dependencies, vtx, {
                rule_t* temp_rule = (rule_t*)graph_get_vertex_value(g, vtx);
                struct stat stat_temp;
                if (stat(temp_rule->target, &stat_temp) == -1 || stat_temp.st_mtime > stat_inf.st_mtime) {
                    flag = 1;
                    break;
                }
            });
            vector_destroy(dependencies);
        }
        if (flag) {
            VECTOR_FOR_EACH(rule->commands, vtx, {
                if (system(vtx) != 0) {
                    done = 0;
                    break;
                }
            });
        }
        //
        pthread_mutex_lock(&mux);
        vector* anti = graph_antineighbors(g, target);
       VECTOR_FOR_EACH(anti, vtx, {
            if (done) {
                rule_t *rule_temp = graph_get_vertex_value(g, vtx);
                rule_temp->state -= 1;
                if (rule_temp->state == 0) queue_push(q, vtx);
            }
            if (!strcmp(vtx, "")) {
                thread_count++;
                pthread_cond_signal(&cond);
            }
        });
        pthread_mutex_unlock(&mux);
        vector_destroy(anti);
    }
    return ptr;
}