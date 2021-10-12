/**
 * malloc
 * CS 241 - Fall 2021
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

static size_t freeSize = 0;

typedef struct meta_data{
    void* ptr;
    size_t size;
    int free;
    struct meta_data *next;
    struct meta_data *prev;
}meta_data;

static meta_data *head = NULL;

void merge(meta_data* toMerge);
void split(meta_data* toSplit, size_t size);

/**
 * Allocate space for array in memory
 *
 * Allocates a block of memory for an array of num elements, each of them size
 * bytes long, and initializes all its bits to zero. The effective result is
 * the allocation of an zero-initialized memory block of (num * size) bytes.
 *
 * @param num
 *    Number of elements to be allocated.
 * @param size
 *    Size of elements.
 *
 * @return
 *    A pointer to the memory block allocated by the function.
 *
 *    The type of this pointer is always void*, which can be cast to the
 *    desired type of data pointer in order to be dereferenceable.
 *
 *    If the function failed to allocate the requested block of memory, a
 *    NULL pointer is returned.
 *
 * @see http://www.cplusplus.com/reference/clibrary/cstdlib/calloc/
 */
void *calloc(size_t num, size_t size) {
    // implement calloc!
    void* temp = malloc(num*size);
    if (!temp){
        return NULL;
    } 
    memset(temp, 0, num*size);
    return temp;
}

/**
 * Allocate memory block
 *
 * Allocates a block of size bytes of memory, returning a pointer to the
 * beginning of the block.  The content of the newly allocated block of
 * memory is not initialized, remaining with indeterminate values.
 *
 * @param size
 *    Size of the memory block, in bytes.
 *
 * @return
 *    On success, a pointer to the memory block allocated by the function.
 *
 *    The type of this pointer is always void*, which can be cast to the
 *    desired type of data pointer in order to be dereferenceable.
 *
 *    If the function failed to allocate the requested block of memory,
 *    a null pointer is returned.
 *
 * @see http://www.cplusplus.com/reference/clibrary/cstdlib/malloc/
 */
void *malloc(size_t size) {
    // implement malloc!
    meta_data* curr = head;
    meta_data* malloc_ = NULL;

    if(freeSize >= size){
        while(curr){
            if(curr->size>=size && curr->free){
                malloc_ = curr;
                freeSize -= size;
                size_t diff = malloc_->size - size;
                if(( diff>= size) && (diff>= sizeof(meta_data))){
                    split(malloc_,size);
                }
                break;
            }
            curr = curr->next;
        }
    }

    if(malloc_){
        malloc_->free = 0;
        return malloc_->ptr;
    }

    malloc_ = sbrk(sizeof(meta_data));
    malloc_->ptr = sbrk(0);
    if(sbrk(size) == (void*) -1){
        return NULL;
    }

    if(head){
        head->prev = malloc_;
    }
    malloc_->next = head;
    malloc_->size = size;
    malloc_->free = 0;
    malloc_->prev = NULL;
    head = malloc_;
    return malloc_->ptr;
}

void split(meta_data* toSplit, size_t size){
    meta_data* new_ = toSplit->ptr + size;
    new_->ptr = new_ + 1;
    new_->size = toSplit->size - size - sizeof(meta_data);
    freeSize += new_->size;
    new_->free = 1;
    toSplit->size = size;
    new_->next = toSplit;
    new_->prev = toSplit->prev;
    if(toSplit->prev){
        toSplit->prev->next = new_;
    }else{
        head = new_;
    }
    toSplit->prev = new_;
    if(new_->prev && new_->prev->free){
        merge(new_);
    }
}


/**
 * Deallocate space in memory
 *
 * A block of memory previously allocated using a call to malloc(),
 * calloc() or realloc() is deallocated, making it available again for
 * further allocations.
 *
 * Notice that this function leaves the value of ptr unchanged, hence
 * it still points to the same (now invalid) location, and not to the
 * null pointer.
 *
 * @param ptr
 *    Pointer to a memory block previously allocated with malloc(),
 *    calloc() or realloc() to be deallocated.  If a null pointer is
 *    passed as argument, no action occurs.
 */

void free(void *ptr) {
    // implement free!
    if (!ptr) {
        return;
    }
    meta_data* temp = ptr - sizeof(meta_data);
    if(temp->free) {
        return;
    }
    temp->free = 1;
    freeSize += (temp->size + sizeof(meta_data));

    if(temp->prev && temp->prev->free == 1) {
        merge(temp);
    }
    if(temp->next && temp->next->free == 1) {
        merge(temp->next);
    }
}

void merge(meta_data* toMerge){
    meta_data* prev_ = toMerge->prev;
    toMerge->size += toMerge->prev->size + sizeof(meta_data);
    if (prev_->prev) {
        prev_->prev->next = toMerge;
        toMerge->prev = prev_->prev;
    }else {
        toMerge->prev = NULL;
        head = toMerge;
    }
}



/**
 * Reallocate memory block
 *
 * The size of the memory block pointed to by the ptr parameter is changed
 * to the size bytes, expanding or reducing the amount of memory available
 * in the block.
 *
 * The function may move the memory block to a new location, in which case
 * the new location is returned. The content of the memory block is preserved
 * up to the lesser of the new and old sizes, even if the block is moved. If
 * the new size is larger, the value of the newly allocated portion is
 * indeterminate.
 *
 * In case that ptr is NULL, the function behaves exactly as malloc, assigning
 * a new block of size bytes and returning a pointer to the beginning of it.
 *
 * In case that the size is 0, the memory previously allocated in ptr is
 * deallocated as if a call to free was made, and a NULL pointer is returned.
 *
 * @param ptr
 *    Pointer to a memory block previously allocated with malloc(), calloc()
 *    or realloc() to be reallocated.
 *
 *    If this is NULL, a new block is allocated and a pointer to it is
 *    returned by the function.
 *
 * @param size
 *    New size for the memory block, in bytes.
 *
 *    If it is 0 and ptr points to an existing block of memory, the memory
 *    block pointed by ptr is deallocated and a NULL pointer is returned.
 *
 * @return
 *    A pointer to the reallocated memory block, which may be either the
 *    same as the ptr argument or a new location.
 *
 *    The type of this pointer is void*, which can be cast to the desired
 *    type of data pointer in order to be dereferenceable.
 *
 *    If the function failed to allocate the requested block of memory,
 *    a NULL pointer is returned, and the memory block pointed to by
 *    argument ptr is left unchanged.
 *
 * @see http://www.cplusplus.com/reference/clibrary/cstdlib/realloc/
 */
void *realloc(void *ptr, size_t size) {
    // implement realloc!
    if(!ptr && size == 0){
        return NULL;
    }
    if(!ptr) {
        return malloc(size);
    }
    if (!size) {
        free(ptr);
        return NULL;
    }

    meta_data* toRealloc = ptr - sizeof(meta_data);
 
    if(toRealloc->size == size){
      return ptr;
    } else if(toRealloc->size > size){
        if(toRealloc->size - size >= sizeof(meta_data)){
            split(toRealloc,size);
            return toRealloc->ptr;
        }
        return ptr;
    } else {
      void *temp = malloc(size);
      if(!temp){
         return NULL;
      }
      memcpy(temp, ptr, toRealloc->size);
      free(ptr);
      return temp;
    }


}


