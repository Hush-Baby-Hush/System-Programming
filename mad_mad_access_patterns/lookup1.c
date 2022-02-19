/**
 * mad_mad_access_patterns
 * CS 241 - Fall 2021
 */
#include "tree.h"
#include "utils.h"
#include "string.h"
#include <stdlib.h>


/*
  Look up a few nodes in the tree and print the info they contain.
  This version uses fseek() and fread() to access the data.

  ./lookup1 <data_file> <word> [<word> ...]
*/

int recursive(FILE* inputfile, char* word, uint32_t offset);

int main(int argc, char **argv) {
  if (argc < 3) {
    printArgumentUsage();
    exit(1);
  }

  char* arg1 = strdup(argv[1]);
  FILE* inputfile = fopen(arg1, "r");
  if (!inputfile) {
    openFail(arg1);
    exit(2);
  }

  char buff[4];
  fread(buff, 1, 4, inputfile);
  int valid;
  if (strcmp(buff, "BTRE") == 0) {
    valid = 1;
  }else valid = 0;

  if (!valid) {
    formatFail(arg1);
    exit(2);
  }

  for (int i = 2; i < argc; i++) {
    recursive(inputfile, argv[i], BINTREE_ROOT_NODE_OFFSET);
  }

  fclose(inputfile);
  free(arg1);
  return 0;
}




int recursive(FILE* inputfile, char* word, uint32_t offset) {

  if (!offset) { 
    printNotFound(word);
    return 0;
  }

  fseek(inputfile, offset, SEEK_SET);
  BinaryTreeNode node;
  fread(&node, sizeof(BinaryTreeNode), 1, inputfile);
  fseek(inputfile, sizeof(BinaryTreeNode) + offset, SEEK_SET);

  char buffer[30];
  fread(buffer, 30, 1, inputfile);

  if (!strcmp(word, buffer)) {
    printFound(buffer, node.count, node.price);
    return 1;
  }
    
  else if (strcmp(word, buffer) > 0) {
    if (recursive(inputfile, word, node.right_child)) {
      return 1;
    }
  }

  else if (strcmp(word, buffer) < 0) {
    if (recursive(inputfile, word, node.left_child)) {
      return 1;
    }
  }

  return 0;
}

