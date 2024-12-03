#ifndef FAT_H
#define FAT_H

#include <stdlib.h>
#include "utils.h"

#define LAST_BLOCK -1
#define BLOCK_EMPTY -2 
#define NO_FREE_BLOCKS -2
#define FREE_BLOCKS_END -5
#define DEFAULT_ISTAKEN 0


struct fat_entry{
 int next;
 int isTaken;
};

int init_fat(struct fat_entry *fat, int* numFreeBlocks, int* freeBlocksHead, int numReservedBlocks);

int add_entry_to_fat(int index,int blockNo, struct fat_entry *fatTbl);
int* get_chain_of_blocks(int head,int* count, struct fat_entry *fatTbl);
int get_nth_block(int head, int n, struct fat_entry *fatTbl);
int free_fat_enteries(int head, struct fat_entry *fatTbl, int* freeBlocksHead, int* numFreeBlocks, int numReservedBlocks);
int get_block_count(int head, struct fat_entry *fatTbl);
int get_free_block(struct fat_entry*fatTbl);

int can_accomodate_n_blocks(int n, int *idx, struct fat_entry *fatTbl, int *freeBlocksHead);   //checks if n blocks can be accomodated. if yes then returns an int* with n free indexes in FAT. 


int reserve_blocks_for_n_size(size_t size, int *numFreeBlocks, int* freeBlocksHead,struct fat_entry* fatTbl);
int can_accomodate_n_size(size_t size,int *numFreeBlocks, struct fat_entry* fatTbl);
int make_free_chain(int *freeBlocksHead, int *numFreeBlocks,
                    struct fat_entry *fatTbl, int numReservedBlocks) ;

void printFatTable(struct fat_entry* fat);

size_t save_fat(struct fat_entry *fat, int fd, off_t offset);
int load_fat(struct fat_entry *fat,int fd, off_t offset);





#endif