#include <stdio.h>
#include <stdlib.h> 

#include "../include/utils.h"


int get_num_blocks(size_t size) {
    if(size > MAX_FILE_SIZE){
        perror(RED"get_num_blocks():Size larger than system can accomodate.");
        return -1;
    }
    return (size + BLOCK_SIZE - 1) / BLOCK_SIZE;
}