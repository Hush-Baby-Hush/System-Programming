/**
 * deadlock_demolition
 * CS 241 - Fall 2021
 */
#include "graph.h"
#include "libdrm.h"
#include "set.h"
#include <pthread.h>

struct drm_t {
    pthread_mutex_t mutex;
};
static graph* g = NULL;
pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;
set* visited;
int cycle_detection(void* input);

drm_t *drm_init() {
    /* Your code here */
    pthread_mutex_lock(&mutex);

    drm_t* init = malloc(sizeof(drm_t));
    pthread_mutex_init(&init->mutex, NULL);

    if(!g){
        g = shallow_graph_create();
    }
    graph_add_vertex(g, init);

    pthread_mutex_unlock(&mutex);

    return init;
}

int drm_post(drm_t *drm, pthread_t *thread_id) {
    /* Your code here */
    pthread_mutex_lock(&mutex);

    if(graph_contains_vertex(g, thread_id) && graph_contains_vertex(g,drm)){
        if(graph_adjacent(g, drm, thread_id)){
            graph_remove_edge(g, drm, thread_id);
            pthread_mutex_unlock(&drm->mutex);
        }
        pthread_mutex_unlock(&mutex);
        return 1; 
    }

    pthread_mutex_unlock(&mutex);
    return 0; 
}

int drm_wait(drm_t *drm, pthread_t *thread_id) {
    /* Your code here */
    pthread_mutex_lock(&mutex);

    graph_add_vertex(g, thread_id);
    if (graph_adjacent(g, drm, thread_id)) {
      pthread_mutex_unlock(&mutex);
      return 0;
    }

    graph_add_edge(g, thread_id, drm);
    if (!cycle_detection(thread_id)) {
        pthread_mutex_unlock(&mutex);

        pthread_mutex_lock(&drm->mutex);
        pthread_mutex_lock(&mutex);
        graph_remove_edge(g, thread_id, drm);
        graph_add_edge(g, drm, thread_id);
        pthread_mutex_unlock(&mutex);
        return 1;
    } else {
        graph_remove_edge(g, thread_id, drm);
        pthread_mutex_unlock(&mutex);
        return 0;
    }

}

void drm_destroy(drm_t *drm) {
    /* Your code here */
    graph_remove_vertex(g, drm);
    pthread_mutex_destroy(&drm->mutex);
    pthread_mutex_destroy(&mutex);    
    free(drm);
    return;
}

int cycle_detection(void* input) {
    if (!visited) {
        visited = shallow_set_create();
    }

    if (set_contains(visited, input)) {
        visited = NULL;
        return 1;
    } else {
        set_add(visited, input);
        vector* neibor = graph_neighbors(g, input);   
        for (size_t i = 0; i< vector_size(neibor); i++) {
            if (cycle_detection(vector_get(neibor, i))) {
                return 1;
            }
        }
        visited = NULL;
        return 0;
    }
}