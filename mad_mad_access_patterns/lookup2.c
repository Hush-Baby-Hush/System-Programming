/**
 * mad_mad_access_patterns
 * CS 241 - Fall 2021
 */
#include "tree.h"
#include "utils.h"
#include "string.h"
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/mman.h>
#include <fcntl.h>
#include <unistd.h>



/*
  Look up a few nodes in the tree and print the info they contain.
  This version uses mmap to access the data.

  ./lookup2 <data_file> <word> [<word> ...]
*/

int recursive(char* addr, char* word, uint32_t offset);

int main(int argc, char **argv) {
  if (argc < 3) {
    printArgumentUsage();
    exit(1);
  }

  char* arg1 = strdup(argv[1]);
  struct stat stat_;
  int fd = open(arg1, O_RDONLY);

  if (fd == -1) {
    openFail(arg1);
    exit(2);
  }

  if (fstat(fd, &stat_) == -1) {
    openFail(arg1);
    exit(2);
  }

  void* addr = mmap(NULL, stat_.st_size, PROT_READ, MAP_PRIVATE, fd, 0);
  if (addr == MAP_FAILED) {
    mmapFail(arg1);
    exit(2);
  }

  int valid;
  if (!strncmp(addr, "BTRE", 4)) {
    valid = 1;
  }else valid = 0;

  if (!valid) {
    formatFail(arg1);
    exit(2);
  }

  for (int i = 2; i < argc; i++) {
    recursive(addr, argv[i], BINTREE_ROOT_NODE_OFFSET);
  }

  close(fd);
  free(arg1);
  return 0;
}


int recursive(char* addr, char* word, uint32_t offset) {
  if (!offset) { 
    printNotFound(word);
    return 0;
  }

  BinaryTreeNode* node = (BinaryTreeNode *) (addr + offset);

  if (!strcmp(word, node->word)) {
    printFound(node->word, node->count, node->price);
    return 1;
  }
    
  else if (strcmp(word, node->word) > 0) {
      if (recursive(addr, word, node->right_child)) {
        return 1;
      }
  }

  else if (strcmp(word, node->word) < 0) {
    if (recursive(addr, word, node->left_child)) {
      return 1;
    }
  }

  return 0;
}