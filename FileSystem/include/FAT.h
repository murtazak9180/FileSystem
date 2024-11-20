#ifndef FAT_H
#define FAT_H

#include <stdlib.h>

#define LAST_BLOCK -1
#define BLOCK_EMPTY -2


struct fat_entry{
 int next;
 int isTaken;
};

int init_fat(struct fat_entry *fat);

int add_entry_to_fat(int index,int blockNo, struct fat_entry *fatTbl);
int* get_chain_of_blocks(int head, struct fat_entry *fatTbl);
int get_nth_block(int head, int n, struct fat_entry *fatTbl);
int free_fat_enteries(int head, struct fat_entry *fatTbl);
int get_block_count(int head, struct fat_entry *fatTbl);
int get_free_block(struct fat_entry*fatTbl);
int reserve_blocks_for_n_size(size_t size, struct fat_entry* fatTbl);
int can_accomodate_n_blocks(int n, int* idx, struct fat_entry* fatTbl);   //checks if n blocks can be accomodated. if yes then returns an int* with n free indexes in FAT. 


int save_fat(struct fat_entry *fat);
int load_fat(struct fat_entry *fat);






#endif