#include <stdio.h>
#include <stdlib.h>

#include "../include/FAT.h"
#include "../include/global.h"
#include "../include/utils.h"

int init_fat(struct fat_entry *fat, int* numFreeBlocks, int* freeBlocksHead) {
  if (!fat) {
    perror(RED "init_fat(): fat is null");
    return -1;
  }
  for (int i = 0; i < MAX_BLOCKS; i++) {
    fat[i].isTaken = DEFAULT_ISTAKEN;
  }
  make_free_chain(freeBlocksHead, numFreeBlocks, fat);
  return 0;
}

int add_entry_to_fat(int index, int blockNo, struct fat_entry *fatTbl) {
  if (fatTbl[index].isTaken) {
    perror(RED "add_entry_to_fat(): index already in use");
    return -1;
  }
  if (!fatTbl) {
    perror(RED "add_entry_to_fat(): Fat Table not allocated");
    return -2;
  }
  fatTbl[index].next = blockNo;
  fatTbl[index].isTaken = 1;
  return 0;
}

int *get_chain_of_blocks(int head, struct fat_entry *fatTbl) {
  int *block_chain;
  int chain_size = 10;
  int count = 0;
  if (!fatTbl) {
    perror(RED "get_chain_of_blocks(): Fat Table not allocated");
    return NULL;
  }
  if (head < 0 || head >= MAX_BLOCKS) {
    perror(RED "get_chain_of_blocks(): invalid head");
    return NULL;
  }
  if ((block_chain = malloc(sizeof(int) * chain_size)) == NULL) {
    perror(RED "get_chain_of_blocks(): malloc()");
    return NULL;
  }

  int current = head;
  while (current != LAST_BLOCK) {
    if (count >= chain_size) {
      chain_size *= 2;
      int *temp = realloc(block_chain, chain_size * sizeof(int));
      if (!temp) {
        free(block_chain);
        perror(RED "get_chain_of_blocks(): realloc() failed");
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
    perror(RED "get_nth_block(): Fat Table not allocated");
    return -1;
  }
  if (head < 0 || head >= MAX_BLOCKS) {
    perror(RED "get_nth_block(): invalid head");
    return -2;
  }
  if (n < 0) {
    perror(RED "get_nth_block(): invalid n");
    return -3;
  }
  if (n == 0) {
    return head;
  }
  current = head;
  while (current != LAST_BLOCK) {
    if (current < 0 || current >= MAX_BLOCKS) {
      perror(RED "get_nth_block(): Corrupted FAT table (invalid block index)");
      return -5;
    }
    current = fatTbl[current].next;
    count++;
    if (count == n) {
      return current;
    }
  }
  perror(RED "get_nth_block(): invalid n");
  return -4;
}


//after freeing, update the freeBlocks chain and 
//update the numFreeBlocks
//update the freeBlocksHead
int free_fat_enteries(int head, struct fat_entry *fatTbl, int* freeBlocksHead, int* numFreeBlocks) {
  int current, next, blockCount, hd;
  blockCount = get_block_count(head, fatTbl);
  if (!fatTbl) {
    perror(RED "get_nth_block(): Fat Table not allocated");
    return -1;
  }
  if (head < 0 || head >= MAX_BLOCKS) {
    perror(RED "get_nth_block(): invalid head");
    return -2;
  }
  current = head;
  while (current != LAST_BLOCK && current != BLOCK_EMPTY) {
    next = fatTbl[current].next;
    fatTbl[current].next = BLOCK_EMPTY;
    fatTbl[current].isTaken = 0;
    current = next;
  }
  if((hd = make_free_chain(freeBlocksHead, numFreeBlocks, fatTbl)) < 0){
    perror(RED"free_fat_enteries(): could not build free chain"RESET);
    return -3;
  }
  

  return 0;

}

int get_block_count(int head, struct fat_entry *fatTbl) {

  int current;
  int count = 1;
  if (!fatTbl) {
    perror(RED "get_nth_block(): Fat Table not allocated");
    return -1;
  }
  if (head < 0 || head >= MAX_BLOCKS) {
    perror(RED "get_nth_block(): invalid head");
    return -2;
  }
  current = head;
  while (current != LAST_BLOCK && current != BLOCK_EMPTY) {
    current = fatTbl[current].next;
    count++;
  }
  return count;
}

int get_free_block(struct fat_entry *fatTbl) {
  if (!fatTbl) {
    perror(RED "get_free_block(): fatTbl is NULL");
    return -1;
  }
  int current = 0;
  while (current < MAX_BLOCKS) {
    if (!fatTbl[current].isTaken) {
      return current;
    }
    current++;
  }
  perror(RED "get_free_block(): No free block.");
  return -2;
}

// checks if n blocks can be accomodated. if yes then populates an int* with n
// free indexes in FAT.
int can_accomodate_n_blocks(int n, int *idx, struct fat_entry *fatTbl) {
  int count = 0;
  if (n <= 0 || n > MAX_BLOCKS) {
    fprintf(stderr, RED "can_accomodate_n_blocks(): Invalid n (%d)\n", n);
    return -1;
  }
  if (!fatTbl) {
    perror(RED "can_accomodate_n_blocks(): Invalid fatTbl");
    return -2;
  }
  if (!idx) {
    perror(RED "can_accomodate_n_blocks(): Invalid idx");
    return -3;
  }
  for (int i = 0; i < MAX_BLOCKS; i++) {
    if (!fatTbl[i].isTaken) {
      idx[count] = i;
      if (count == n) {
        return 0;
      }
      count++;
    }
  }

  fprintf(stderr,
          RED
          "can_accomodate_n_blocks(): Only %d free blocks available, need %d\n",
          count, n);
  return -4;
}

// int reserve_blocks_for_n_size(size_t size, struct fat_entry *fatTbl) {
//   int numBlocks, head;
//   int *idx;
//   if (size <= 0 || size > MAX_FILE_SIZE) {
//     fprintf(stderr, RED "reserve_blocks_for_n_size(): Invalid size (%zu)\n",
//             size);
//     return -1;
//   }
//   if (!fatTbl) {
//     perror(RED "reserve_blocks_for_n_size(): Invalid fatTbl");
//     return -2;
//   }
//   if ((numBlocks = get_num_blocks(size)) < 0) {
//     fprintf(stderr,
//             RED "reserve_blocks_for_n_size(): Failed to calculate numBlocks "
//                 "for size (%zu)\n",
//             size);
//     return -3;
//   }

//   if ((idx = malloc(sizeof(int) * numBlocks)) == NULL) {
//     perror(RED "reserve_blocks_for_n_size(): malloc()");
//     return -4;
//   }
//   if (can_accomodate_n_blocks(numBlocks, idx, fatTbl) < 0) {
//     fprintf(stderr,
//             RED "reserve_blocks_for_n_size(): Cannot reserve %d blocks\n",
//             numBlocks);
//     return -5;
//   }

//   // idx[0] will be at the firstBlock entry in the directory table
//   // from idx[1] onwards, we need to populate FAT.
//   for (int i = 0; i < numBlocks - 1;
//        i++) { // we will fill the next the last block with LAST_BLOCK hence
//               // numBlocks - 1
//     fatTbl[idx[i]].next = idx[i + 1];
//     fatTbl[idx[i]].isTaken = 1;
//   }
//   fatTbl[idx[numBlocks - 1]].next = LAST_BLOCK;
//   fatTbl[idx[numBlocks - 1]].isTaken = 1;
//   head = idx[0];
//   free(idx);
//   return head; // return the head for directory table.
// }

// int can_accomodate_n_size(size_t size, struct fat_entry *fatTbl) {

//   if (size <= 0 || size > MAX_FILE_SIZE) {
//     fprintf(stderr,
//             RED "can_accomodate_n_size(): Invalid size (%zu). Must be between
//             "
//                 "1 and %d.\n",
//             size, MAX_FILE_SIZE);
//     return -1;
//   }

//   if (!fatTbl) {
//     perror(RED "can_accomodate_n_size(): Invalid fatTbl");
//     return -2;
//   }

//   int requiredBlocks = get_num_blocks(size);
//   if (requiredBlocks <= 0) {
//     fprintf(stderr,
//             RED "can_accomodate_n_size(): Failed to calculate required blocks
//             "
//                 "for size (%zu).\n",
//             size);
//     return -3;
//   }
//   int freeBlocks = 0;

//   // Count the number of free blocks in FAT
//   for (int i = 0; i < MAX_BLOCKS; i++) {
//     if (!fatTbl[i].isTaken) {
//       freeBlocks++;
//       if (freeBlocks >= requiredBlocks) {
//         return 0;
//       }
//     }
//   }

//   fprintf(stderr,
//           RED
//           "can_accomodate_n_size(): Only %d free blocks available, need
//           %d.\n", freeBlocks, requiredBlocks);
//   return -4;
// }

int can_accomodate_n_size(size_t size, int *numFreeBlocks,
                          struct fat_entry *fatTbl) {
  if (size <= 0 || size > MAX_FILE_SIZE) {
    fprintf(stderr,
            RED "can_accomodate_n_size(): Invalid size (%zu). Must be between "
                "1 and %d.\n",
            size, MAX_FILE_SIZE);
    return -1;
  }
  if (!fatTbl) {
    perror(RED "can_accomodate_n_size(): Invalid fatTbl");
    return -2;
  }
  if (!numFreeBlocks) {
    perror(RED "can_accomodate_n_size(): numFreeBlocks null");
    return -3;
  }

  int requiredBlocks = get_num_blocks(size);
  if (requiredBlocks <= 0) {
    fprintf(stderr,
            RED "can_accomodate_n_size(): Failed to calculate required blocks "
                "for size (%zu).\n",
            size);
    return -4;
  }

  return (requiredBlocks < *numFreeBlocks) ? 0 : -5;
}
int make_free_chain(int *freeBlocksHead, int *numFreeBlocks,
                    struct fat_entry *fatTbl) {
  int free = 0;
  int prev = -1;
  int isHead = 1;
  int temp = *freeBlocksHead;
  
  *freeBlocksHead = NO_FREE_BLOCKS;
  if (!fatTbl) {
    perror(RED "make_free_chain(): fatTbl null");
    *freeBlocksHead = temp; 
    return -1;
  }
  for (int i = 0; i < MAX_BLOCKS; i++) {
    if (!fatTbl[i].isTaken && fatTbl[i].next != LAST_BLOCK) {
      free++;
      if (prev != -1) {
        fatTbl[prev].next = i;
      }
      if (isHead) {
        *freeBlocksHead = i;
        isHead = 0;
      }
      prev = i;
    }
  }
  if (prev != -1) {
    fatTbl[prev].next = FREE_BLOCKS_END;
  }

  if (free == 0) {
    perror("make_free_chain(): no free blocks");
    *freeBlocksHead = NO_FREE_BLOCKS;
  }
  *numFreeBlocks = free;
  return 0;
}


//TEST!
int reserve_blocks_for_n_size(size_t size, int *numFreeBlocks,
                              int *freeBlocksHead, struct fat_entry *fatTbl) {
  int reqBlocks, head;
  int count = 1;
  if (size <= 0 || size > MAX_FILE_SIZE) {
    fprintf(stderr, RED "reserve_blocks_for_n_size(): Invalid size (%zu)\n",
            size);
    return -1;
  }
  if (!fatTbl) {
    perror(RED "reserve_blocks_for_n_size(): Invalid fatTbl");
    return -2;
  }
  if (!numFreeBlocks || !freeBlocksHead) {
    perror(RED "reserve_blocks_for_n_size(): Inavlid head or numFreeBlocks");
    return -3;
  }

  if (can_accomodate_n_size(size, numFreeBlocks, fatTbl) < 0) {
    perror(RED
           "reserve_blocks_for_n_size(): cannot accomodate the required size");
    return -4;
  }
  reqBlocks = get_num_blocks(size);
  if (reqBlocks < 0) {
    perror(RED "reserve_blocks_for_n_size(): get_num_blocks()");
    return -5;
  }
  head = *freeBlocksHead;

  *numFreeBlocks = *numFreeBlocks - reqBlocks;
  for (int i = head; count <= reqBlocks;
       count++) { // TODO: correct the logic. Also put the last block as
                  // LAST_BLOCK.

    if (i >=
        0) { // Also mark the blocks as is taken! Return head(the current head)
      *freeBlocksHead = fatTbl[i].next;
      if (count <= reqBlocks) {
        fatTbl[i].isTaken = 1;
      }
      if (count == reqBlocks) {
        fatTbl[i].next = LAST_BLOCK;
      }
      i = fatTbl[i].next;
    } 
  }
  return head; 
}

void printFatTable(struct fat_entry* fat) {
    printf("\nFAT Table:\n");
    printf("| Index | Next | IsTaken |\n");
    printf("|-------|------|---------|\n");

    for (int i = 0; i < MAX_BLOCKS; i++) {
        printf("| %-5d | %-4d | %-7d |\n", i, fat[i].next, fat[i].isTaken);
    }
}