#include <stdio.h>
#include <stdlib.h>

#include "../include/DirectoryTable.h"
#include "../include/FAT.h"
#include "../include/utils.h"

struct directory_entry dirTable[DIR_ENTERIES];
struct fat_entry fat[MAX_BLOCKS];

// we will use param here if we do the bonus parts
int initialze() {
  if (init_dir_table(dirTable) < 0) {
    perror(RED "initialize(): can not init dirTable");
    return -1;
  }
  if (init_fat(fat) < 0) {
    perror(RED "initialize(): can not init fat");
    return -2;
  }
  return 0;
}

int main() {
  if (initialze() < 0) {
    perror(RED "initialze()");
    return -1;
  }

  return 0;
}