#ifndef DIRECTORYTABLE_H
#define DIRECTORYTABLE_H

#include <stdlib.h>

#include "FAT.h"
#include "utils.h"

#define ROOT_DIR "ROOT"
#define FREE  "FREE"
#define DEFAULT_SIZE -1
#define DEFAULT_FIRST_BLOCK -1
#define DEFAULT_ISDIR -1
#define DEFAULT_ISTAKEN 0
#define DEFAULT_NAME '\0'
#define DEFAULT_PARENT '\0'

struct directory_entry{
  char name[MAX_FILE_NAME_SIZE];
  char parentDir[MAX_FILE_NAME_SIZE];
  size_t size;
  int firstBlock;
  int isDir;
  int isTaken;
};

int init_dir_table(struct directory_entry* dirTable);

int add_entry(const char **names, const int numEnteries, size_t size, int isDir, struct directory_entry *dirTable, struct fat_entry *fat);
struct directory_entry *search_entry(const char **names, int isDir,struct directory_entry *dirTable);
int delete_entry(const char **names,const int numEnteries,int isDir, struct directory_entry *dirTable);
int get_first_block(const char **names,struct directory_entry *dirTable);

int save_dir_table(struct directory_entry *dirTable);
int load_dir_table(struct directory_entry *dirTable);

#endif