/**
 * critical_concurrency
 * CS 241 - Fall 2021
 */
#include "queue.h"
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>

/**
 * This queue is implemented with a linked list of queue_nodes.
 */
typedef struct queue_node {
    void *data;
    struct queue_node *next;
} queue_node;

struct queue {
    /* queue_node pointers to the head and tail of the queue */
    queue_node *head, *tail;

    /* The number of elements in the queue */
    ssize_t size;

    /**
     * The maximum number of elements the queue can hold.
     * max_size is non-positive if the queue does not have a max size.
     */
    ssize_t max_size;

    /* Mutex and Condition Variable for thread-safety */
    pthread_cond_t cv;
    pthread_mutex_t m;
};

queue *queue_create(ssize_t max_size) {
    /* Your code here */
    queue* this = malloc(sizeof(queue));
    if (!this) {
        return NULL;
    }
    this->head = NULL;
    this->tail = NULL;
    this->size = 0;
    this->max_size = max_size;
    pthread_cond_init(&(this->cv), NULL);
    pthread_mutex_init(&(this->m), NULL);
    return this;
}

void queue_destroy(queue *this) {
    /* Your code here */
    if (!this) {
        return;
    }
    queue_node* curr = this->head;
    queue_node* temp;
    while (curr) {
        temp = curr;
        curr = curr->next;
        free(temp);
    }
    pthread_mutex_destroy(&(this->m));
    pthread_cond_destroy(&(this->cv));
    free(this);
}

void queue_push(queue *this, void *data) {
    /* Your code here */
    pthread_mutex_lock(&this->m);

    while (this->max_size > 0 && this->size == this->max_size) {
      pthread_cond_wait(&this->cv, &this->m);
    }
    queue_node *push = malloc(sizeof(queue_node));
    push->data = data;
    push->next = NULL;

    if (!this->size) {
        this->head = push;
        this->tail = push;
    } else {
        this->tail->next = push;
        this->tail = push;
    }

    this->size++;

    if (this->size > 0) {
        pthread_cond_broadcast(&this->cv);
    }
    pthread_mutex_unlock(&this->m);
}

void *queue_pull(queue *this) {
    /* Your code here */
    pthread_mutex_lock(&(this->m));
    while (!this->head) {
        pthread_cond_wait(&this->cv, &this->m);

    }
    queue_node* pull = this->head;
    void* data_ = pull->data;

    if (this->head == this->tail) {
        this->head = NULL;
        this->tail = NULL;
    } else {
        this->head = pull->next;
    }
    free(pull);

    this->size--;
    if (this->size > 0 && this->size < this->max_size) {
        pthread_cond_broadcast(&(this->cv));
    }
    pthread_mutex_unlock(&(this->m));
    return data_;
}
