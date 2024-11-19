#ifndef FAT_H
#define FAT_H

#define LAST_BLOCK -1
#define BLOCK_EMPTY -2


struct fat_entry{
 int next;
 int isTaken;
};

int add_entry_to_fat(int index,int blockNo, struct fat_entry *fatTbl);
int* get_chain_of_blocks(int head, struct fat_entry *fatTbl);
int get_nth_block(int head, int n, struct fat_entry *fatTbl);
int free_fat_enteries(int head, struct fat_entry *fatTbl);
int get_block_count(int head, struct fat_entry *fatTbl);





#endif