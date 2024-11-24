#ifndef OPERATIONS_H
#define OPERATIONS_H
#include "DirectoryTable.h"
#include "FAT.h"
#include "hashmap.h"
#include <unistd.h>

int create_directory_or_file(char **temp, int numEnteries, size_t size,int isDir, struct directory_entry* dirTable, struct fat_entry* fat, int* numFreeBlocks, int* freeBlocksHead, struct HashMap**map);
int delete_directory_or_file(char** temp, int numEnteries, int isDir,struct directory_entry* dirTable, struct fat_entry* fat, int* numFreeBlocks, int* freeBlocksHead, struct HashMap**map);
int write_to_file();
int read_from_file();
int list_directory();
int truncate_file();

#endif