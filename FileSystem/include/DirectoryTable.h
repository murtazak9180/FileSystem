#ifndef DIRECTORYTABLE_H
#define DIRECTORYTABLE_H

#include <stdlib.h>

#include "utils.h"

struct directory_entry{
  char name[MAX_FILE_NAME_SIZE];
  size_t size;
  int firstBlock;
  int isDir;
  int isTaken;
};

int add_entry(const char* name, size_t size,int isDir,struct directory_entry* dirTable);
struct directory_entry* search_entry(const char* name,struct directory_entry *dirTable);
int delete_entry(const char* name,struct directory_entry *dirTable);
int get_first_block(const char*name);
int is_directory(const char* name);

#endif