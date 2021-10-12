/**
 * teaching_threads
 * CS 241 - Fall 2021
 */
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "reduce.h"
#include "reducers.h"

/* You might need a struct for each task ... */
typedef struct threads{
    int* list;
    size_t len;
    int base_case;
    reducer reduce_func;
    
}threads;

/* You should create a start routine for your threads. */

void* start(void* data){
    threads* new_ = (threads*)data;
	int d = new_->base_case;
	for (size_t i = 0; i < new_->len; i++) {
        d = new_->reduce_func(d, new_->list[i]);
    }
    int * temp = (int*)malloc(sizeof(int));
    *temp = d;
    return (void*) temp;

}


int par_reduce(int *list, size_t list_len, reducer reduce_func, int base_case,
               size_t num_threads) {
    /* Your implementation goes here */
    if(list_len <= num_threads){
        return reduce(list,list_len,reduce_func,base_case);
    } 
    int splt = list_len / num_threads;
    pthread_t threads_[num_threads];
    int nlist[num_threads];
    threads* arr[num_threads];
    size_t i;
    for(i=0; i < num_threads-1; i++){
        arr[i] = (threads*)malloc(sizeof(threads));
        arr[i] -> list = list + (i*splt);
        arr[i] -> len = splt;
        arr[i] -> base_case = base_case;
        arr[i] -> reduce_func = reduce_func;
    }
    arr[i] = (threads*)malloc(sizeof(threads));
    arr[i] -> list = list + (i*splt);
    arr[i] -> len = list_len - (i*splt);
    arr[i] -> base_case = base_case;
    arr[i] -> reduce_func = reduce_func;

    for(size_t j = 0; j < num_threads; j++){
        pthread_create(&threads_[j], 0, start, (void*)arr[j]);
    }

    for(size_t k = 0; k < num_threads; k++){
        void* temp;
        pthread_join(threads_[k],&temp);
        nlist[k] = *((int*)temp);
        free(temp);
        free(arr[k]);
    }
    int ans = reduce(nlist, num_threads, reduce_func, base_case);
    return ans;


}
