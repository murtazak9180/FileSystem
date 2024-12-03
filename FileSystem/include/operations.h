#ifndef OPERATIONS_H
#define OPERATIONS_H
#include "DirectoryTable.h"
#include "FAT.h"
#include "hashmap.h"
#include <unistd.h>

int create_directory_or_file(char **temp, int numEnteries, size_t size,int isDir, struct directory_entry* dirTable, struct fat_entry* fat, int* numFreeBlocks, int* freeBlocksHead, struct HashMap**map);
int delete_directory_or_file(char **temp, int numEnteries, int isDir,
                             struct directory_entry *dirTable,
                             struct fat_entry *fat, int *numFreeBlocks,
                             int *freeBlocksHead, struct HashMap **map, int numReservedBlocks);
int write_to_file(char **temp, int numEnteries, int isDir,
                  struct directory_entry *dirTable, struct fat_entry *fat,
                  int *numFreeBlocks, int *freeBlocksHead,
                  struct HashMap **map, int numReservedBlocks) ;
//the caller is suppsoed to free readBuf after they are done using it. 
int read_from_file(char** temp, int numEnteries,char **readBuf, int isDir,struct directory_entry* dirTable, struct fat_entry* fat, int* numFreeBlocks, int* freeBlocksHead, struct HashMap**map);
int list_directory(char** temp, int numEnteries,struct directory_entry* dirTable, struct HashMap**map);
int truncate_file(char **temp, int numEnteries, size_t numBytes, int isDir,
                  int *numFreeBlocks, int *freeBlocksHead,
                  struct directory_entry *dirTable, struct fat_entry *fat,
                  struct HashMap **map, int numReservedBlocks) ;
int read_from_file_offset(char **temp, int numEntries, char **readBuf, int isDir,
                          struct directory_entry *dirTable, struct fat_entry *fat,
                          int *numFreeBlocks, int *freeBlocksHead,
                          struct HashMap **map, size_t offset, size_t bytesToRead) ;

#endif