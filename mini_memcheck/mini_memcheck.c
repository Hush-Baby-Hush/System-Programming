/**
 * mini_memcheck
 * CS 241 - Fall 2021
 */
#include "mini_memcheck.h"
#include <stdio.h>
#include <string.h>

meta_data* head = NULL;
size_t total_memory_requested = 0;
size_t total_memory_freed = 0;
size_t invalid_addresses = 0;


void *mini_malloc(size_t request_size, const char *filename,
                  void *instruction) {
    // your code here

    if(request_size<=0) 
    {
        return NULL;
    }
    meta_data* mem = (meta_data*) malloc(sizeof(meta_data) + request_size);
    if(!mem) 
    {
        return NULL;
    }
    mem->request_size = request_size;
    mem->filename = filename;
    mem-> instruction = instruction;
    mem-> next = head;
    head = mem;
    total_memory_requested += request_size;
    return (void*)(mem+1);

}

void *mini_calloc(size_t num_elements, size_t element_size,
                  const char *filename, void *instruction) {

    size_t size = num_elements * element_size;
    void* calloc_space = mini_malloc(size, filename, instruction);
    if(calloc_space)
    {
        memset(calloc_space,0,size);
    }
    return calloc_space;
}

void *mini_realloc(void *payload, size_t request_size, const char *filename,
                   void *instruction) {
    
    if(!payload) 
    {
        return mini_malloc(request_size, filename, instruction);
    }

    if(!request_size) 
    {
        mini_free(payload);
        return NULL;
    }
    
    meta_data* prev = NULL;
    meta_data* curr = head;
    meta_data* next;
    
    while(curr) 
    {
        next = curr->next; 
        void* mem = ((void*) curr) + sizeof(meta_data);
        if (mem == payload) 
        {
            if(request_size == curr->request_size) 
            {
                curr->filename = filename;
                curr->instruction = instruction;

                return payload; 
            }

            if (prev) 
            {
                prev -> next = next;
            } else 
            {
                head = next;
            }
            if (request_size > curr -> request_size) 
            {
                total_memory_requested += (request_size - curr -> request_size);
            } else if (request_size < curr -> request_size) 
            {
                total_memory_freed += (curr -> request_size - request_size );
            }

            void* mem = realloc(curr, sizeof(meta_data) + request_size);
            if (!mem) 
            {
                return NULL;
            }
            meta_data * new = (meta_data *) mem;
            void * required = mem + sizeof(meta_data);
            new -> request_size = request_size; 
            new -> filename = filename; 
            new -> instruction = instruction; 
            new -> next = head; 
            head = new; 
            return required;
        } 

            prev = curr;
            curr = next;
    }
    invalid_addresses++;
    return NULL;
}

void mini_free(void *payload) {
    if(!payload) 
    {
        return;
    }
    meta_data* meta = ((meta_data*)payload - 1);
    meta_data* curr = head;
    meta_data* prev = NULL;
    while(curr)
    {
        if(curr == meta)
        {
            if(prev)
            {
                prev->next = curr->next;
            }else
            {
                head = curr->next;
            }
            total_memory_freed += curr->request_size;
            free(curr);
            return;
        }
        prev = curr;
        curr = curr->next;
    }
    invalid_addresses++;
}
