#include <stdio.h>
#include <stdlib.h>

#include "../include/FAT.h"

int add_entry_to_fat(int index, int blockNo, struct fat_entry *fatTbl) {
  if (fatTbl[index].isTaken) {
    perror("add_entry_to_fat(): index already in use");
    return -1;
  }
  if (!fatTbl) {
    perror("add_entry_to_fat(): Fat Table not allocated");
    return -2;
  }
  fatTbl[index].next = blockNo;
  fatTbl[index].isTaken = 1;
  fatTbl[blockNo].next = LAST_BLOCK; // it is the last block for now.
  return 0;
}

int *get_chain_of_blocks(int head, struct fat_entry *fatTbl) {
  int *block_chain;
  int chain_size = 10;
  int count = 0;
  if (!fatTbl) {
    perror("get_chain_of_blocks(): Fat Table not allocated");
    return NULL;
  }
  if (head < 0) {
    perror("get_chain_of_blocks(): invalid head");
    return NULL;
  }
  if ((block_chain = malloc(sizeof(int) * chain_size)) == NULL) {
    perror("get_chain_of_blocks(): malloc()");
    return NULL;
  }

  int current = head;
  while (current != LAST_BLOCK) {
    if (count >= chain_size) {
      chain_size *= 2;
      int *temp = realloc(block_chain, chain_size * sizeof(int));
      if (!temp) {
        free(block_chain);
        perror("get_chain_of_blocks(): realloc() failed");
        return NULL;
      }
      block_chain = temp;
    }
    block_chain[count++] = current;
    current = fatTbl[current].next;
  }

  int *final_chain = realloc(block_chain, count * sizeof(int));
  if (!final_chain) {
    return block_chain;
  }
  return final_chain;
}

int get_nth_block(int head, int n, struct fat_entry *fatTbl){
    
}