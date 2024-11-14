#ifndef DIRECTORYTABLE_H
#define DIRECTORYTABLE_H

#include <stdlib.h>

struct directory_entry{
  char name[64];
  size_t size;
  int firstBlock;
  int isDir;
  int isTaken;
};

int add_entry(const char* name, size_t size,int isDir,struct directory_entry* dirTable);
struct directory_entry* search_entry(const char* name);
int delete_entry(const char* entry);
int get_first_block(const char*name);
int is_directory(const char* name);

#endif