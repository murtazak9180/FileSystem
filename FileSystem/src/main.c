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
#include "../include/operations.h"

int numFreeBlocks;
int freeBlocksHead;

struct directory_entry dirTable[DIR_ENTERIES];
struct fat_entry fat[MAX_BLOCKS];
struct HashMap *map;

// we will use param here if we do the bonus parts
int initialze() {
  numFreeBlocks = MAX_BLOCKS;
  freeBlocksHead = 0;
  if (init_dir_table(dirTable) < 0) {
    perror(RED "initialize(): can not init dirTable");
    return -1;
  }
  if (init_fat(fat, &numFreeBlocks, &freeBlocksHead) < 0) {
    perror(RED "initialize(): can not init fat");
    return -2;
  }
  map = NULL;
  return 0;
}

// TODO: FREE tokenizedinput
int main() {
  char *line = NULL;
  size_t len = 0;
  int numEnteries = 0;
  size_t allocatedSize = 10;
  char **temp = NULL;
  int head_idx = -1;
  int idx = -1;
  if (initialze() < 0) {
    perror(RED "initialze()");
    return -1;
  }
  while (1) {
    char *tokenizedInput[allocatedSize];
    if (getline(&line, &len, stdin) == -1) {
      perror(RED "getline(): Error reading input" RESET);
    }
    size_t length = strlen(line);
    if (length > 0 && line[length - 1] == '\n') {
      line[length - 1] = '\0';
    }

    if ((numEnteries = tokenize_input(line, tokenizedInput, &allocatedSize)) <=
        0) {
      perror(RED "Could not tokenize" RESET);
      free(line);
    }
    free(line);
    if (numEnteries > 0) {
      if (strcmp(tokenizedInput[0], MAKE_DIR) == 0) {
        // make directory
        temp = tokenizedInput + 1;
        create_directory_or_file(temp, numEnteries - 1, 0, 1, dirTable, fat, &numFreeBlocks,
                  &freeBlocksHead, &map);

      } else if (strcmp(tokenizedInput[0], MAKE_FILE) == 0) {
        // make file
        temp = tokenizedInput + 1;
        create_directory_or_file(temp, numEnteries - 1, 0, 0, dirTable, fat, &numFreeBlocks,
                  &freeBlocksHead, &map);
      } else if (strcmp(tokenizedInput[0], DELETE_DIR) == 0) {
        // delete directory
        temp = tokenizedInput + 1;
        delete_directory_or_file(temp, numEnteries-1, 1, dirTable, fat, &numFreeBlocks,&freeBlocksHead, &map);

      } else if (strcmp(tokenizedInput[0], DELETE_FILE) == 0) {
        // delete file
        temp = tokenizedInput + 1;
        delete_directory_or_file(temp, numEnteries-1, 0, dirTable, fat, &numFreeBlocks,&freeBlocksHead, &map);

      } else if (strcmp(tokenizedInput[0], LIST) == 0) {
        // list
      } else if (strcmp(tokenizedInput[0], READ_FILE) == 0) {
        // read file
      } else if (strcmp(tokenizedInput[0], WRITE_FILE) == 0) {
        // write file
        write_to_file(temp, numEnteries-1,0,dirTable,fat,&numFreeBlocks, &freeBlocksHead,&map);

      } else if (strcmp(tokenizedInput[0], TRUNCATE_FILE) == 0) {
        // truncate file
      }
      printDirectoryTable(dirTable);
      printFatTable(fat);
      print_map(map);
    }
  }

  return 0;
}