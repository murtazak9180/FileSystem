#include <stdio.h>
#include <stdlib.h> 
#include <string.h>

#include "../include/utils.h"


int get_num_blocks(size_t size) {
    if(size > MAX_FILE_SIZE){
        perror(RED"get_num_blocks():Size larger than system can accomodate.");
        return -1;
    }
    return (size + BLOCK_SIZE - 1) / BLOCK_SIZE;
}

char *construct_path(char **names, int count, int isDir) {
    int totalLength = 0;
    for (int i = 0; i < count; i++) {
        totalLength += strlen(names[i]);
        if (i < count - 1) {
            totalLength += 1;
        }
    }

    if (isDir) {
        totalLength += 1; 
    }
    totalLength += 1; 

    char *path = malloc(totalLength);
    if (!path) {
        perror("malloc failed");
        return NULL;
    }

    path[0] = '\0';
    for (int i = 0; i < count; i++) {
        strcat(path, names[i]);
        if (i < count - 1) {
            strcat(path, "/");
        }
    }

    // Append trailing '/' if isDir is true
    if (isDir) {
        strcat(path, "/");
    }

    return path;
}