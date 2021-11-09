/**
 * finding_filesystems
 * CS 241 - Fall 2021
 */
#include "minixfs.h"
#include "minixfs_utils.h"
#include <errno.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <time.h>
#include <stdlib.h>


/**
 * Virtual paths:
 *  Add your new virtual endpoint to minixfs_virtual_path_names
 */
char *minixfs_virtual_path_names[] = {"info", /* add your paths here*/};

/**
 * Forward declaring block_info_string so that we can attach unused on it
 * This prevents a compiler warning if you haven't used it yet.
 *
 * This function generates the info string that the virtual endpoint info should
 * emit when read
 */
static char *block_info_string(ssize_t num_used_blocks) __attribute__((unused));
static char *block_info_string(ssize_t num_used_blocks) {
    char *block_string = NULL;
    ssize_t curr_free_blocks = DATA_NUMBER - num_used_blocks;
    asprintf(&block_string,
             "Free blocks: %zd\n"
             "Used blocks: %zd\n",
             curr_free_blocks, num_used_blocks);
    return block_string;
}


// Don't modify this line unless you know what you're doing
int minixfs_virtual_path_count =
    sizeof(minixfs_virtual_path_names) / sizeof(minixfs_virtual_path_names[0]);

int minixfs_chmod(file_system *fs, char *path, int new_permissions) {
    // Thar she blows!
    if (!get_inode(fs, path)) {
        errno = ENOENT; 
        return -1;
    }
    inode* inode = get_inode(fs, path);
    uint16_t temp = inode->mode >> RWX_BITS_NUMBER;
    inode->mode = new_permissions | (temp << RWX_BITS_NUMBER);
    clock_gettime(CLOCK_REALTIME, &(inode->ctim));
    return 0;
}

int minixfs_chown(file_system *fs, char *path, uid_t owner, gid_t group) {
    // Land ahoy!
    if (!get_inode(fs, path)) {
        errno = ENOENT; 
        return -1;
    }
    inode *inode = get_inode(fs, path);
    if (owner != ((uid_t)-1)){
        inode->uid = owner;
    }
    if (group != ((gid_t)-1)){
        inode->gid = group;
    }
    clock_gettime(CLOCK_REALTIME, &(inode->ctim));
    return 0;
}

inode *minixfs_create_inode_for_path(file_system *fs, const char *path) {
    // Land ahoy!
    if (valid_filename(path) == 1 || get_inode(fs, path)) {
        return NULL;
    }

    const char* file = NULL;
    inode* parent = parent_directory(fs, path, &file);

    if (!parent || !is_directory(parent)) {
        return NULL;
    }

    inode_number finode = first_unused_inode(fs);
    if (finode == -1) {
        return NULL;
    }
    inode* new_node = fs->inode_root + finode;
    init_inode(parent, new_node);
    minixfs_dirent dirent;
    dirent.name = (char*) file;
    dirent.inode_num = finode;

    int mod = parent->size % sizeof(data_block);
    int div = parent->size / sizeof(data_block);
    if ((!mod && add_data_block_to_inode(fs, parent) == -1) || (div >= NUM_DIRECT_BLOCKS)) {
        return NULL;
    }
    data_block_number* result;
    int temp = div;
    if(temp >= NUM_DIRECT_BLOCKS){
        result = (data_block_number*)(fs->data_root + parent->indirect);
        temp -= NUM_DIRECT_BLOCKS;
    } 
    else{
       result = parent->direct;
    }

    void* start = (void*) (fs->data_root + result[temp]) + mod;
    memset(start, 0, sizeof(data_block));
    make_string_from_dirent(start, dirent);
    parent->size += MAX_DIR_NAME_LEN;
    return new_node;
}



ssize_t minixfs_virtual_read(file_system *fs, const char *path, void *buf,
                             size_t count, off_t *off) {
    if (!strcmp(path, "info")) {
        // TODO implement the "info" virtual file here
        ssize_t used = 0;
        char* data = GET_DATA_MAP(fs->meta);
        for(uint64_t i = 0; i < fs->meta->dblock_count; i++) {
            if (data[i] == 1) {
                used++;
            }
        }
        char* str = block_info_string(used);
        if (*off > (off_t)strlen(str)) {
            return 0;
        }
        if (count > strlen(str) - *off) {
            count = strlen(str) - *off;
        }
        memmove(buf, str + *off, count);
        *off += count;
        return count;
    }

    errno = ENOENT;
    return -1;
}

ssize_t minixfs_write(file_system *fs, const char *path, const void *buf,
                      size_t count, off_t *off) {
    // X marks the spot
    
    if (!get_inode(fs, path)) {
        inode* inode = minixfs_create_inode_for_path(fs, path);
        if (!inode) {
            errno = ENOSPC;
            return -1;
        }
    }

    uint64_t max = (NUM_DIRECT_BLOCKS + NUM_INDIRECT_BLOCKS) * sizeof(data_block);
    int block = (count + *off + sizeof(data_block) - 1) / sizeof(data_block);
    if (minixfs_min_blockcount(fs, path, block) == -1 || count + *off > max) {
        errno = ENOSPC;
        return -1;
    }

    uint64_t div = *off / sizeof(data_block);
    size_t mod = *off % sizeof(data_block);
    uint64_t len;
    if (count + mod > sizeof(data_block)) {
        len = sizeof(data_block) - mod;
    } else {
        len = count;
    }

    inode* inode = get_inode(fs, path);
    data_block_number* result;
    uint64_t temp = div;
    if(temp >= NUM_DIRECT_BLOCKS){
        result = (data_block_number*)(fs->data_root + inode->indirect);
        temp -= NUM_DIRECT_BLOCKS;
    } else {
        result = inode->direct;
    }

    void * memblock = (void*) (fs->data_root + result[temp]) + mod;
    memcpy(memblock, buf, len);
    *off += len;
    div++;
    uint64_t wrcount = len;
    while (wrcount < count) {
        if (count - wrcount < sizeof(data_block)) {
            len = count - wrcount;     
        } else {
            len = sizeof(data_block);
        }

        uint64_t temp2 = div;
        if(temp2 >= NUM_DIRECT_BLOCKS){
            result = (data_block_number*)(fs->data_root + inode->indirect);
            temp2 -= NUM_DIRECT_BLOCKS;
        } else {
            result = inode->direct;
        }
        memblock = (void*) (fs->data_root + result[temp2]);
        memcpy(memblock, buf + wrcount, len);
        div++;
        *off += len;
        wrcount += len;
    }
    if(count + *off > inode->size){
        inode->size  = count + *off;
    }

    clock_gettime(CLOCK_REALTIME, &inode->atim);
    clock_gettime(CLOCK_REALTIME, &inode->mtim);
    return wrcount;
}





ssize_t minixfs_read(file_system *fs, const char *path, void *buf, size_t count,
                     off_t *off) {
    const char *virtual_path = is_virtual_path(path);
    if (virtual_path)
        return minixfs_virtual_read(fs, virtual_path, buf, count, off);
    // 'ere be treasure!
    if (!buf || !get_inode(fs, path)) {
        errno = ENOENT;
        return -1;
    }
    inode* inode = get_inode(fs, path);

    if ((uint64_t)*off > inode->size) {
        return 0;
    }
    if (count > inode->size - *off) {
        count = inode->size - *off;
    }
    size_t mod = *off % sizeof(data_block);
    uint64_t size;
    if (count + mod > sizeof(data_block)) {
        size = sizeof(data_block) - mod;
    } 
    else {
        size = count;
    }

    uint64_t div = *off / sizeof(data_block);
    data_block_number* result;
    uint64_t temp = div;
    if(temp >= NUM_DIRECT_BLOCKS){
        result = (data_block_number*)(fs->data_root + inode->indirect);
        temp -= NUM_DIRECT_BLOCKS;
    } 
    else {
        result = inode->direct;
    }
    void* memblock = (void*) (fs->data_root + result[temp]) + mod;
    memcpy(buf, memblock, size);
    *off += size;
    div++;

    uint64_t readcount = size;
    while (readcount < count) {
        if(count - readcount < sizeof(data_block)){
            size = count - readcount;      
        }
        else{
            size = sizeof(data_block);
        }
        uint64_t temp2 = div;
        if(temp2 >= NUM_DIRECT_BLOCKS){
            result = (data_block_number*)(fs->data_root + inode->indirect);
            temp2 -= NUM_DIRECT_BLOCKS;
        } 
        else {
            result = inode->direct;
        }
        memblock = (void*) (fs->data_root + result[temp2]);
        memcpy(buf + readcount, memblock, size);
        div ++;
        *off += size;
        readcount += size;
    }
    clock_gettime(CLOCK_REALTIME, &inode->atim);
    return readcount;
}

