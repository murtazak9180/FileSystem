#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "../include/utils.h"
#include "../include/hashmap.h"
#include "../include/FAT.h"
#include "../include/DirectoryTable.h"


int get_num_blocks(size_t size) {
  if (size == 0) {
    return 0;
  }
  return (size + BLOCK_SIZE - 1) / BLOCK_SIZE;
}

char *construct_path(char **names, int count, int isDir) {
  int totalLength = 0;
  for (int i = 0; i < count; i++) {
    totalLength += strlen(names[i]);
    if (i < count - 1) {
      totalLength += 1;
    }
  }

  if (isDir) {
    totalLength += 1;
  }
  totalLength += 1;

  char *path = malloc(totalLength);
  if (!path) {
    perror("malloc failed");
    return NULL;
  }

  path[0] = '\0';
  for (int i = 0; i < count; i++) {
    strcat(path, names[i]);
    if (i < count - 1) {
      strcat(path, "/");
    }
  }

  // Append trailing '/' if isDir is true
  if (isDir) {
    strcat(path, "/");
  }

  return path;
}

void printDirectory(const char *parentDirectory, char **children,
                    int numChildren, int depth) {

  for (int i = 0; i < depth; i++) {
    printf("    ");
  }
  printf( CYAN"\n└── %s\n"RESET, parentDirectory);

  
  for (int i = 0; i < numChildren; i++) {
    for (int j = 0; j <= depth; j++) {
      printf("    "); 
    }
    printf(CYAN"└── %s\n"RESET, children[i]);
  }
}

int num_metadata_blks_req(){
  size_t gdtSize = sizeof(struct directory_entry) * DIR_ENTERIES;
  size_t fatSize = sizeof(struct fat_entry) * MAX_BLOCKS;
  // size_t mapSize = (MAX_FILE_NAME_SIZE/*The max file size*/+ sizeof(int)/*size of key*/+ sizeof(unsigned int) /*size of length of key*/) * DIR_ENTERIES;
  // mapSize+=sizeof(unsigned int);  //for the numEnteries. 
  size_t others = sizeof(int) * 3 ; // numFreeBlocks, freeBlocksHead, numBlocksReserved

  return get_num_blocks(gdtSize+fatSize+others+1);
}