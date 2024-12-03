#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "../include/DirectoryTable.h"
#include "../include/FAT.h"
#include "../include/hashmap.h"
#include "../include/utils.h"

// WHile init, make isDir = -1, size = 0, firstBlock = -1

int init_dir_table(struct directory_entry *dirTable) {
  if (!dirTable) {
    perror(RED "init_dir_table(): dirTable is NULL");
    return -1;
  }
  for (int i = 0; i < DIR_ENTERIES; i++) {
    dirTable[i].isDir = DEFAULT_ISDIR;
    dirTable[i].size = DEFAULT_SIZE;
    dirTable[i].firstBlock = DEFAULT_FIRST_BLOCK;
    dirTable[i].isTaken = DEFAULT_ISTAKEN;
    dirTable[i].parentIdx = DEFAULT_PARENT;
    memset(dirTable[i].name, DEFAULT_NAME, MAX_FILE_NAME_SIZE);
  }
  return 0;
}

int add_entry(char **names, const int numEnteries, size_t size, int isDir,
              struct directory_entry *dirTable, struct fat_entry *fat,
              int *numFreeBlocks, int *freeBlocksHead, struct HashMap **map) {
  int head, parentIdx;
  char *path;
  if (!names || !names[0]) {
    perror(RED "add_entry(): Name is empty");
    return -1;
  }
  if (numEnteries <= 0) {
    perror(RED "add_entry(): Invalid numEnteries");
    return -2;
  }

  if (size > MAX_FILE_SIZE) {
    perror(RED "add_entry(): Size exceeds max file size");
    return -3;
  }
  // if(!map){
  //    perror(RED "add_entry(): Hashmap is null");
  //    return -4;
  // }
  if (strlen(names[numEnteries - 1]) >
      MAX_FILE_NAME_SIZE) { // check if the last component in the path
                            // voilates the size limits.
    perror(RED "add_entry(): Max file name size exceeded");
    return -4;
  }

  if (search_entry(names, numEnteries, isDir, map) > -1) {
    perror(RED "add_entry(): File/Directory already exists ");
    return -5;
  }
  if (numEnteries > 1) { // i.e not in the root directory
    char **temp = malloc(sizeof(char *) * (numEnteries - 1));
    if (!temp) {
      perror(RED "add_entry(): Memory allocation failed for temp");
      return -6;
    }

    // Copy the path components excluding the last one
    for (int i = 0; i < numEnteries - 1; i++) {
      temp[i] = names[i];
    }

    // Ensure the parent directory exists
    if ((parentIdx = search_entry(temp, numEnteries - 1, 1, map)) < 0) {
      perror(RED "add_entry(): Invalid parent directory");
      free(temp);
      return -7;
    }

    free(temp);
  }
  if (!isDir) {
    if (can_accomodate_n_size(size, numFreeBlocks, fat) < 0) {
      fprintf(stderr,
              RED "add_entry(): Insufficient space for the given size: %zu",
              size);
      return -8;
    }
  }

  // if the entry already does not exit and the path preceeding the entry is
  // indeed valid then find a free entry in dirTable and fill it with the
  // required component.
  for (int i = 0; i < DIR_ENTERIES; i++) {
    if (dirTable[i].isTaken == 0) {
      if (isDir) {
        dirTable[i].firstBlock = -1;
      } else {
        if ((head = reserve_blocks_for_n_size(size, numFreeBlocks,
                                              freeBlocksHead, fat)) < 0 &&
            head != DEFAULT_FIRST_BLOCK) {
          fprintf(stderr, RED "add_entry(): could not return a valid head");
          return -9;
        } else {
          dirTable[i].firstBlock = head;
        }
      }
      strncpy(dirTable[i].name, names[numEnteries - 1], MAX_FILE_NAME_SIZE - 1);
      dirTable[i].name[MAX_FILE_NAME_SIZE - 1] = '\0';
      dirTable[i].size = size;
      dirTable[i].isDir = isDir;
      dirTable[i].isTaken = 1;
      if (numEnteries > 1) {

        dirTable[i].parentIdx = parentIdx;
      } else {
        dirTable[i].parentIdx = DEFAULT_PARENT;
      }
      path = construct_path(names, numEnteries, isDir);
      if (!path) {
        perror(RED "add_entry(): invalid path");
        return -11;
      }
      printf("Before adding to map, map = %p\n", (void *)map);
      add_to_map(map, path, i);
      printf("After adding to map, map = %p\n", (void *)map);
      free(path); // add the key(path) and the value(index in GDT) to hashmap
      printf("After free path, map = %p\n", (void *)map);
      return 0;
    }
  }

  perror(RED "add_entry(): No free space in directory table");
  return -10;
}

int search_entry(char **names, int numEnteries, int isDir,
                 struct HashMap **map) {
  char *path;
  int idx = -1;
  if (!names || !names[0]) {
    perror(RED "search_entry(): Name is null");
    return -1;
  }
  if (numEnteries <= 0) {
    perror(RED "search_entry(): invalid numEnteries");
    return -2;
  }
  // if(!map){
  //   perror(RED"search_entry(): HashMap is null");
  //   return -3;
  // }
  path = construct_path(names, numEnteries, isDir);
  if (!path) {
    perror(RED "search_entry(): invalid path");
    return -4;
  }
  if ((idx = get_value(*map, path)) < 0) {
    perror(RED "search_entry(): Path does not exist");
    free(path);
    return -5;
  }
  free(path);
  return idx;
}

int delete_entry(char **names, const int numEnteries, int isDir,
                 struct directory_entry *dirTable, struct HashMap **map) {
  int idx;
  struct directory_entry *de;
  char *path;
  if (!names || !names[0]) {
    perror(RED "delete_entry(): Names is NULL\n");
    return -1;
  }

  if (numEnteries <= 0) {
    perror(RED "delete_entry(): Invalid numEnteries");
    return -2;
  }
  // if(!map){
  //   perror(RED "delete_entry(): Invalid HashMap");
  //   return -3;
  // }

  if ((idx = search_entry(names, numEnteries, isDir, map)) < 0) {
    perror(RED "delete_entry(): Invalid Path");
    return -3;
  }
  if (isDir) { // if it is a directory, it cannot be deleted if it is a parent
               // directory of any component
    for (int i = 0; i < DIR_ENTERIES; i++) {
      if (dirTable[i].parentIdx == idx && dirTable[i].isTaken) {
        perror(RED "delete_entry():The given directory has children "
                   "components, CANNOT BE DELETED.");
        return -4;
      }
    }
  }
  de = &dirTable[idx];

  memset(de->name, DEFAULT_NAME, MAX_FILE_NAME_SIZE);
  de->parentIdx = DEFAULT_PARENT;
  de->size = DEFAULT_SIZE;
  de->firstBlock = DEFAULT_FIRST_BLOCK;
  de->isDir = DEFAULT_ISDIR;
  de->isTaken = DEFAULT_ISTAKEN;
  path = construct_path(names, numEnteries, isDir);
  if (!path) {
    perror(RED "delete_entry(): Invalid Path(Null)");
    return -5;
  }
  delete_from_map(map, path);
  return 0;
}

int get_first_block(char **names, int numEnteries,
                    struct directory_entry *dirTable, struct HashMap **map) {
  int idx;
  if (!names || names[0]) {
    perror(RED "get_first_block(): Name is NULL\n");
    return -1;
  }
  // if(!map){
  //   perror(RED "get_first_block(): Invalid Map\n");
  //   return -2;
  // }
  if ((idx = search_entry(names, numEnteries, 0, map)) <
      0) { // no block assigned to a directory
    perror(RED "get_first_block(): Entry does not exist");
    return -2;
  }
  return dirTable[idx].firstBlock;
}

void printDirectoryTable(struct directory_entry *dirTable) {
  printf("Directory Table:\n");
  printf("| Index | Name                          | ParentIdx | Size   | "
         "FirstBlock | IsDir | IsTaken |\n");
  printf("|-------|--------------------------------|-----------|--------|------"
         "------|-------|---------|\n");

  for (int i = 0; i < DIR_ENTERIES; i++) {
    printf("| %-5d | %-30s | %-9d | %-6zu | %-10d | %-5d | %-7d |\n", i,
           dirTable[i].name, dirTable[i].parentIdx, dirTable[i].size,
           dirTable[i].firstBlock, dirTable[i].isDir, dirTable[i].isTaken);
  }
}

int get_children(int idx, char ***children, int *count,
                 struct directory_entry *dirTable) {
  *count = 0;
  if ((idx < 0 || idx > DIR_ENTERIES) && idx != DEFAULT_PARENT) {
    perror(RED "get_children(): invalid idx" RESET);
    return -1;
  }
  if (!dirTable) {
    perror(RED "get_children(): null dirTable" RESET);
    return -2;
  }
  if (!*children || !*children[0]) {
    perror(RED "get_children(): null children" RESET);
    return -3;
  }

  for (int i = 0; i < DIR_ENTERIES; i++) {
    if (dirTable[i].parentIdx == idx && dirTable[i].isTaken) {
      memcpy((*children)[*count], dirTable[i].name, MAX_FILE_NAME_SIZE);
      (*count)++;
    }
  }
  return 0;
}
ssize_t save_dir_table(struct directory_entry *dirTable, int fd, off_t offset) {
  ssize_t bytesWritten = 0;
  ssize_t totalBytes = 0;

  if (!dirTable) {
    perror(RED "save_dir_table(): dirTable is null" RESET);
    return -1;
  }

  for (int i = 0; i < DIR_ENTERIES; i++) {
    // Write each field individually
    bytesWritten = pwrite(fd, dirTable[i].name, MAX_FILE_NAME_SIZE, offset);
    if (bytesWritten < 0) {
      perror(RED "save_dir_table(): write failed for name" RESET);
      return -1;
    }
    if ((size_t)bytesWritten != MAX_FILE_NAME_SIZE) {
      fprintf(stderr, RED "save_dir_table(): incomplete write for name" RESET);
      return -1;
    }
    totalBytes += bytesWritten;
    offset += MAX_FILE_NAME_SIZE;

    bytesWritten = pwrite(fd, &dirTable[i].parentIdx, sizeof(int), offset);
    if (bytesWritten < 0) {
      perror(RED "save_dir_table(): write failed for parentIdx" RESET);
      return -1;
    }
    totalBytes += bytesWritten;
    offset += sizeof(int);

    bytesWritten = pwrite(fd, &dirTable[i].size, sizeof(size_t), offset);
    if (bytesWritten < 0) {
      perror(RED "save_dir_table(): write failed for size" RESET);
      return -1;
    }
    totalBytes += bytesWritten;
    offset += sizeof(size_t);

    bytesWritten = pwrite(fd, &dirTable[i].firstBlock, sizeof(int), offset);
    if (bytesWritten < 0) {
      perror(RED "save_dir_table(): write failed for firstBlock" RESET);
      return -1;
    }
    totalBytes += bytesWritten;
    offset += sizeof(int);

    bytesWritten = pwrite(fd, &dirTable[i].isDir, sizeof(int), offset);
    if (bytesWritten < 0) {
      perror(RED "save_dir_table(): write failed for isDir" RESET);
      return -1;
    }
    totalBytes += bytesWritten;
    offset += sizeof(int);

    bytesWritten = pwrite(fd, &dirTable[i].isTaken, sizeof(int), offset);
    if (bytesWritten < 0) {
      perror(RED "save_dir_table(): write failed for isTaken" RESET);
      return -1;
    }
    totalBytes += bytesWritten;
    offset += sizeof(int);
  }

  return totalBytes;
}

int load_dir_table(struct directory_entry *dirTable, int fd, off_t offset) {
  ssize_t bytesRead = 0;

  if (!dirTable) {
    perror(RED "load_dir_table(): dirTable is null" RESET);
    return -1;
  }

  for (int i = 0; i < DIR_ENTERIES; i++) {
    // Read each field individually
    bytesRead = pread(fd, dirTable[i].name, MAX_FILE_NAME_SIZE, offset);
    if (bytesRead < 0) {
      perror(RED "load_dir_table(): read failed for name" RESET);
      return -1;
    }
    if ((size_t)bytesRead != MAX_FILE_NAME_SIZE) {
      fprintf(stderr, RED "load_dir_table(): incomplete read for name" RESET);
      return -1;
    }
    offset += MAX_FILE_NAME_SIZE;

    bytesRead = pread(fd, &dirTable[i].parentIdx, sizeof(int), offset);
    if (bytesRead < 0) {
      perror(RED "load_dir_table(): read failed for parentIdx" RESET);
      return -1;
    }
    offset += sizeof(int);

    bytesRead = pread(fd, &dirTable[i].size, sizeof(size_t), offset);
    if (bytesRead < 0) {
      perror(RED "load_dir_table(): read failed for size" RESET);
      return -1;
    }
    offset += sizeof(size_t);

    bytesRead = pread(fd, &dirTable[i].firstBlock, sizeof(int), offset);
    if (bytesRead < 0) {
      perror(RED "load_dir_table(): read failed for firstBlock" RESET);
      return -1;
    }
    offset += sizeof(int);

    bytesRead = pread(fd, &dirTable[i].isDir, sizeof(int), offset);
    if (bytesRead < 0) {
      perror(RED "load_dir_table(): read failed for isDir" RESET);
      return -1;
    }
    offset += sizeof(int);

    bytesRead = pread(fd, &dirTable[i].isTaken, sizeof(int), offset);
    if (bytesRead < 0) {
      perror(RED "load_dir_table(): read failed for isTaken" RESET);
      return -1;
    }
    offset += sizeof(int);
  }

  return 0;
}
