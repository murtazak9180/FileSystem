#include <stdio.h>
#include <stdlib.h>

#include "../include/DirectoryTable.h"
#include "../include/utils.h"

int add_entry(const char name[], size_t size, int isDir,
              struct directory_entry *dirTable) {
  struct directory_entry de;
  if (!name) {
    perror("add_entry(): Name is empty");
    return -1;
  }

  if (size > MAX_FILE_SIZE) {
    perror("add_entry(): Size exceeds max file size");
    return -2;
  }

  for (int i = 0; i < DIR_ENTERIES; i++) {
    if (dirTable[i].isTaken == 0) {
      if ((de = malloc(sizeof(struct directory_entry))) == NULL) {
        perror("add_entry(): malloc() failed");
        return -3;
      }


    }
  }
}
