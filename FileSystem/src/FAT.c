#include <stdio.h>
#include <stdlib.h>

#include "../include/FAT.h"
#include "../include/utils.h"

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
  if (head < 0 || head >= MAX_BLOCKS) {
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

int get_nth_block(int head, int n, struct fat_entry *fatTbl) {
  int current;
  int count = 0;
  if (!fatTbl) {
    perror("get_nth_block(): Fat Table not allocated");
    return -1;
  }
  if (head < 0 || head >= MAX_BLOCKS) {
    perror("get_nth_block(): invalid head");
    return -2;
  }
  if (n < 0) {
    perror("get_nth_block(): invalid n");
    return -3;
  }
  if (n == 0) {
    return head;
  }
  current = head;
  while (current != LAST_BLOCK) {
    if (current < 0 || current >= MAX_BLOCKS) {
      perror("get_nth_block(): Corrupted FAT table (invalid block index)");
      return -5;
    }
    current = fatTbl[current].next;
    count++;
    if (count == n) {
      return current;
    }
  }
  perror("get_nth_block(): invalid n");
  return -4;
}

int free_fat_enteries(int head, struct fat_entry *fatTbl) {
  int current, next;
  if (!fatTbl) {
    perror("get_nth_block(): Fat Table not allocated");
    return -1;
  }
  if (head < 0 || head >= MAX_BLOCKS) {
    perror("get_nth_block(): invalid head");
    return -2;
  }
  current = head;
  while (current != LAST_BLOCK && current != BLOCK_EMPTY) {
    next = fatTbl[current].next;
    fatTbl[current].next = BLOCK_EMPTY;
    fatTbl[current].isTaken = 0;
    current = next;
  }
  return 0;
}

int get_block_count(int head, struct fat_entry *fatTbl) {
  int current;
  int count = 1;
  if (!fatTbl) {
    perror("get_nth_block(): Fat Table not allocated");
    return -1;
  }
  if (head < 0 || head >= MAX_BLOCKS) {
    perror("get_nth_block(): invalid head");
    return -2;
  }
  current = head;
  while (current != LAST_BLOCK && current != BLOCK_EMPTY) {
    current = fatTbl[current].next;
    count++;
  }
}