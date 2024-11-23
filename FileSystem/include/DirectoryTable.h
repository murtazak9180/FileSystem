#ifndef DIRECTORYTABLE_H
#define DIRECTORYTABLE_H

#include <stdlib.h>

#include "FAT.h"
#include "utils.h"
#include "hashmap.h"

#define ROOT_DIR "ROOT"
#define FREE  "FREE"
#define DEFAULT_SIZE 0
#define DEFAULT_FIRST_BLOCK -1
#define DEFAULT_ISDIR -1
#define DEFAULT_ISTAKEN 0
#define DEFAULT_NAME '\0'
#define DEFAULT_PARENT -2


//TODO: change the parentDir to index of the parentDir in the GDT rather than the name
//
struct directory_entry{
  char name[MAX_FILE_NAME_SIZE];
  int parentIdx;
  size_t size;
  int firstBlock;
  int isDir;
  int isTaken;
};

int init_dir_table(struct directory_entry* dirTable);

int add_entry(char **names, const int numEnteries, size_t size, int isDir,struct directory_entry *dirTable, struct fat_entry *fat,int *numFreeBlocks,int* freeBlocksHead, struct HashMap** map);
int search_entry(char **names, int numEnteries,int isDir,struct HashMap** map);
int delete_entry(char **names, const int numEnteries, int isDir,struct directory_entry *dirTable, struct HashMap**map);
int get_first_block(char **names, int numEnteries,struct directory_entry *dirTable, struct HashMap** map);
void printDirectoryTable(struct directory_entry* dirTable);

int save_dir_table(struct directory_entry *dirTable);
int load_dir_table(struct directory_entry *dirTable);

#endif