#include <stdio.h>
#include <stdlib.h>

#include "../include/DirectoryTable.h"
#include "../include/FAT.h"
#include "../include/disk.h"
#include "../include/global.h"
#include "../include/hashmap.h"
#include "../include/parse.h"
#include "../include/uthash.h"
#include "../include/utils.h"
int create_directory_or_file(char **temp, int numEnteries, size_t size,
                             int isDir, struct directory_entry *dirTable,
                             struct fat_entry *fat, int *numFreeBlocks,
                             int *freeBlocksHead, struct HashMap **map) {
  add_entry(temp, numEnteries, size, isDir, dirTable, fat, numFreeBlocks,
            freeBlocksHead, map);
  return 0;
}

int delete_directory_or_file(char **temp, int numEnteries, int isDir,
                             struct directory_entry *dirTable,
                             struct fat_entry *fat, int *numFreeBlocks,
                             int *freeBlocksHead, struct HashMap **map) {
  int idx = -1;
  int head_idx = -1;
  if ((idx = search_entry(temp, numEnteries, isDir, map)) < 0) {
    perror(RED "Directory does not exist" RESET);
    return -1;
  }
  if (idx > 0) {
    head_idx = dirTable[idx].firstBlock;
    free_fat_enteries(head_idx, fat, freeBlocksHead, numFreeBlocks);
  }
  delete_entry(temp, numEnteries, isDir, dirTable, map);
  return 0;
}

size_t greater_min_smaller(size_t a, size_t b) {
  return (a > b) ? a - b : b - a;
}

int write_to_file(char **temp, int numEnteries, int isDir,
                  struct directory_entry *dirTable, struct fat_entry *fat,
                  int *numFreeBlocks, int *freeBlocksHead,
                  struct HashMap **map) {
  int idx = -1;
  char *writeBuf = NULL;
  size_t bytesToWrite = -1;
  size_t currSize = -1;
  int currBlock = -1;
  int head = -1;
  size_t off = 0;
  size_t diskOffset;
  size_t len = 0; 
  if ((idx = search_entry(temp, numEnteries, isDir, map)) < 0) {
    perror(RED "write_to_file():The target file does not exits" RESET);
    return -1;
  }
  if (dirTable[idx].isDir) {
    perror(RED "write_to_file(): Target is a directory, cannot write" RESET);
    return -2;
  }
  currSize = dirTable[idx].size;
  printf(YELLOW
         "\nEnter the data you want to write(end with enter key press)\n" GREEN);
  if (getline(&writeBuf, &bytesToWrite, stdin) == -1) {
    perror(RED "write_to_file(): getline()" RESET);
    return -3;
  }
  len = strlen(writeBuf);
  if (currSize == 0) {
    if (can_accomodate_n_size(len, numFreeBlocks, fat) < 0) {
      perror(RED "write_to_file(): Entered data is too large, cannot be "
                 "accomodated" RESET);
      return -4;
    }
    free_fat_enteries(dirTable[idx].firstBlock, fat, freeBlocksHead, numFreeBlocks);
  } else if (currSize > 0) {

    if (currSize < len) {
      if (can_accomodate_n_size(len - currSize, numFreeBlocks, fat) <
          0) {
        perror(RED "write_to_file(): Entered data is too large, cannot be "
                   "accomodated" RESET);
        return -5;
      }
    }
    currBlock = dirTable[idx].firstBlock;
    while (currSize > 0 && currBlock >= 0) {
      if (currSize < BLOCK_SIZE) {
        truncate_at_offset(currSize, currBlock * BLOCK_SIZE);
        currSize = currSize - currSize;
      } else if (currSize > BLOCK_SIZE) {
        truncate_at_offset(BLOCK_SIZE, currBlock * BLOCK_SIZE);
        currSize -= BLOCK_SIZE;
      }
      currBlock = fat[currBlock].next;
    }

    free_fat_enteries(dirTable[idx].firstBlock, fat, freeBlocksHead,
                      numFreeBlocks);
  }
  if ((head = reserve_blocks_for_n_size(len, numFreeBlocks,
                                        freeBlocksHead, fat)) < 0) {
    perror(RED "write_to_file(): Could not reserve blocks");
    return -6;
  }
  dirTable[idx].size = len;
  dirTable[idx].firstBlock = head;

  while (off < len && head >= 0) {
    diskOffset = head * BLOCK_SIZE;
    size_t chunkSize =
        (len - off) < BLOCK_SIZE ? (len - off) : BLOCK_SIZE;
    char *chunk = (char *)malloc(chunkSize);
    if (chunk == NULL) {
      perror(RED "write_to_file(): chunk is null" RESET);
      return -7;
    }
    memcpy(chunk, writeBuf + off, chunkSize);
    write_to_disk(chunk, chunkSize, diskOffset);

    off += chunkSize;
    head = fat[head].next;
    free(chunk);
  }
  return 0; 
}