#ifndef UTILS_H
#define UTILS_H

#include <stdlib.h>

#define MAX_FILE_SIZE 131072  //128 blocks * blocksize 
#define BLOCK_SIZE 1024
#define MAX_FILE_NAME_SIZE 64
#define DIR_ENTERIES 128
#define MAX_BLOCKS 65536


#define RESET	"\x1b[0m"
#define RED		"\x1b[31m"
#define GREEN	"\x1b[32m"
#define YELLOW	"\x1b[33m"
#define BLUE	"\x1b[34m"
#define MAGENTA "\x1b[35m"
#define CYAN	"\x1b[36m"
#define WHITE	"\x1b[37m"


int get_num_blocks(size_t size);

#endif