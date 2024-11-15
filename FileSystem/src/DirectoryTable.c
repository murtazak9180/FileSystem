#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "../include/DirectoryTable.h"
#include "../include/utils.h"

// TODO: findfreeblock()
int add_entry(const char *name, size_t size, int isDir,
              struct directory_entry *dirTable) {
  if (!name) {
    perror("add_entry(): Name is empty");
    return -1;
  }

  if (size > MAX_FILE_SIZE) {
    perror("add_entry(): Size exceeds max file size");
    return -2;
  }

  if (search_entry(name, dirTable) != NULL) {
    perror("add_entry(): File/Directory already exists");
    return -3;
  }

  for (int i = 0; i < DIR_ENTERIES; i++) {
    if (dirTable[i].isTaken == 0) {
      strncpy(dirTable[i].name, name, MAX_FILE_NAME_SIZE - 2);
      dirTable[i].name[MAX_FILE_NAME_SIZE - 1] = '\0';
      dirTable[i].size = size;
      dirTable[i].isDir = isDir;
      // dirTable[i].firstBlock = findfreeblock()
      dirTable[i].isTaken = 1;
      return 0;
    }
  }

  perror("add_entry(): No free space in directory table");
  return -4;
}

struct directory_entry *search_entry(const char *name,
                                     struct directory_entry *dirTable) {
  if (!name || name[0] == '\0') {
    perror("search_entry(): Name is NULL\n");
    return NULL;
  }

  for (int i = 0; i < DIR_ENTERIES; i++) {
    if (dirTable[i].isTaken && strcmp(dirTable[i].name, name) == 0) {
      return &dirTable[i];
    }
  }
  return NULL;
}

int delete_entry(const char *name, struct directory_entry *dirTable) {
  struct directory_entry *de;
  if (!name || name[0] == '\0') {
    perror("search_entry(): Name is NULL\n");
    return -1;
  }
  if ((de = search_entry(name, dirTable)) == NULL) {
    perror("delete_entry(): Entry does not exist");
    return -2;
  }

  memset(de->name, 0, MAX_FILE_NAME_SIZE);
  de->size = 0;
  de->firstBlock = -1;
  de->isDir = 0;
  de->isTaken = 0;

  return 0;
}