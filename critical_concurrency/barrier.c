/**
 * critical_concurrency
 * CS 241 - Fall 2021
 */
#include "barrier.h"

// The returns are just for errors if you want to check for them.
int barrier_destroy(barrier_t *barrier) {
    int error = 0;
    pthread_mutex_destroy(&(barrier->mtx));
    pthread_cond_destroy(&(barrier->cv));
    return error;
}

int barrier_init(barrier_t *barrier, unsigned int num_threads) {
    int error = 0;

    pthread_mutex_init(&barrier->mtx, NULL);
    pthread_cond_init(&barrier->cv, NULL);

    barrier->count = 0;
    barrier->times_used = 0;
    barrier->n_threads = num_threads;

    return error;
}

int barrier_wait(barrier_t *barrier) {
    pthread_mutex_lock(&barrier->mtx);
    barrier->count++;
    if (barrier->n_threads == barrier->count) {
      pthread_cond_broadcast(&barrier->cv);
    }
    while (barrier->count < barrier->n_threads) {
      pthread_cond_wait(&barrier->cv, &barrier->mtx);
    }

    pthread_mutex_unlock(&barrier->mtx);
    pthread_mutex_lock(&barrier->mtx);
    barrier->times_used++;

    if (barrier->count == barrier->times_used) {
      barrier->times_used = 0;
      barrier->count = 0;
      pthread_cond_broadcast(&barrier->cv);
    }

    while (barrier->times_used) {
      pthread_cond_wait(&barrier->cv, &barrier->mtx);
    }
    pthread_mutex_unlock(&barrier->mtx);
    return 0;
}
