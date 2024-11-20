#ifndef DIRECTORYTABLE_H
#define DIRECTORYTABLE_H

#define ROOT_DIR "ROOT"
#define FREE  "FREE"

#include <stdlib.h>

#include "utils.h"

struct directory_entry{
  char name[MAX_FILE_NAME_SIZE];
  char parentDir[MAX_FILE_NAME_SIZE];
  size_t size;
  int firstBlock;
  int isDir;
  int isTaken;
};

int init_dir_table(struct directory_entry* dirTable);

int add_entry(const char **names,const int numEnteries, size_t size, int isDir, struct directory_entry *dirTable);
struct directory_entry *search_entry(const char **names, int isDir,struct directory_entry *dirTable);
int delete_entry(const char **names,const int numEnteries,int isDir, struct directory_entry *dirTable);
int get_first_block(const char **names,struct directory_entry *dirTable);

int save_dir_table(struct directory_entry *dirTable);
int load_dir_table(struct directory_entry *dirTable);

#endif