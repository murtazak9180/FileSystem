#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "../include/DirectoryTable.h"
#include "../include/utils.h"
#include "../include/FAT.h"

//WHile init, make isDir = -1, size = 0, firstBlock = -1


// TODO: findfreeblock()
int add_entry(const char **names, const int numEnteries, size_t size, int isDir,
              struct directory_entry *dirTable) {
  if (!names || !names[0]) {
    perror(RED"add_entry(): Name is empty");
    return -1;
  }
  if (numEnteries <= 0) {
    perror(RED"add_entry(): Invalid numEnteries");
    return -6;
  }

  if (size > MAX_FILE_SIZE) {
    perror(RED"add_entry(): Size exceeds max file size");
    return -2;
  }
  if (strlen(names[numEnteries - 1]) >
      MAX_FILE_NAME_SIZE - 1) { // check if the last component in the path
                                // voilates the size limits.
    perror(RED"add_entry(): Max file name size exceeded");
    return -5;
  }

  if (search_entry(names, isDir, dirTable) != NULL) {
    perror(RED"add_entry(): File/Directory already exists ");
    return -3;
  }
  if (numEnteries > 1) { // i.e not in the root directory
    const char **temp = malloc(sizeof(char *) * (numEnteries - 1));
    if (!temp) {
      perror(RED"add_entry(): Memory allocation failed for temp");
      return -7;
    }

    // Copy the path components excluding the last one
    for (int i = 0; i < numEnteries - 1; i++) {
      temp[i] = names[i];
    }

    // Ensure the parent directory exists
    if (search_entry(temp, 1, dirTable) == NULL) {
      perror(RED"add_entry(): Invalid parent directory");
      free(temp);
      return -4;
    }

    free(temp);
  }

  // if the entry already does not exit and the path preceeding the entry is
  // indeed valid then find a free entry in dirTable and fill it with the
  // required component.
  for (int i = 0; i < DIR_ENTERIES; i++) {
    if (dirTable[i].isTaken == 0) {
      strncpy(dirTable[i].name, names[numEnteries - 1], MAX_FILE_NAME_SIZE - 1);
      dirTable[i].name[MAX_FILE_NAME_SIZE - 1] = '\0';
      dirTable[i].size = size;
      dirTable[i].isDir = isDir;
      if(isDir){
        dirTable[i].firstBlock = -1;
      }else{
      // dirTable[i].firstBlock = findfreeblock()
      }
      dirTable[i].isTaken = 1;
      if (numEnteries > 1) {
        strncpy(dirTable[i].parentDir, names[numEnteries - 2],
                MAX_FILE_NAME_SIZE - 1);
        dirTable[i].parentDir[MAX_FILE_NAME_SIZE - 1] = '\0';
      } else {
        strncpy(dirTable[i].parentDir, ROOT_DIR, MAX_FILE_NAME_SIZE - 1);
        dirTable[i].parentDir[MAX_FILE_NAME_SIZE - 1] = '\0';
      }
      return 0;
    }
  }

  perror(RED"add_entry(): No free space in directory table");
  return -4;
}

struct directory_entry *search_entry(const char **names, int isDir,
                                     struct directory_entry *dirTable) {
  if (!names || !names[0]) {
    perror(RED"search_entry(): Path components are NULL or empty");
    return NULL;
  }

  struct directory_entry *current = NULL;

  for (int i = 0; names[i]; i++) {
    const char *currentName = names[i];
    const char *parentDir = (i > 0) ? current->name : ROOT_DIR;

    // Search for the current component in the directory table
    struct directory_entry *next = NULL;
    for (int j = 0; j < DIR_ENTERIES; j++) {
      if (dirTable[j].isTaken && strcmp(dirTable[j].name, currentName) == 0 &&
          strcmp(dirTable[j].parentDir, parentDir) == 0) {
        next = &dirTable[j];
        break;
      }
    }

    if (!next) {
      perror(RED"search_entry(): Component not found in the path");
      return NULL;
    }

    // If this is the last component, verify its type (file or directory)
    if (!names[i + 1]) {
      if (isDir && !next->isDir) {
        perror(RED"search_entry(): Target is not a directory");
        return NULL;
      }
      if (!isDir && next->isDir) {
        perror(RED"search_entry(): Target is not a file");
        return NULL;
      }
      return next;
    }

    // Ensure intermediate components are directories
    if (!next->isDir) {
      perror(RED"search_entry(): Intermediate path component is not a directory");
      return NULL;
    }

    current = next; 
  }

  perror(RED"search_entry(): Unexpected error");
  return NULL;
}

int delete_entry(const char **names,const int numEnteries,int isDir, struct directory_entry *dirTable) {
  struct directory_entry *de;  
  if (!names || !names[0]) {
    perror(RED"delete_entry(): Names is NULL\n");
    return -1;
  }

  if(numEnteries<= 0){
    perror(RED"delete_entry(): Invalid numEnteries");
    return -2; 
  }
  
  if((de=search_entry(names, isDir, dirTable)) == NULL){
    perror(RED"delete_entry(): Invalid Path");
    return -3; 
  }
  if(isDir){   //if it is a directory, it cannot be deleted if it is a parent directory of any component
    for(int i = 0; i<DIR_ENTERIES; i++){
      if(strncmp(dirTable[i].parentDir,names[numEnteries-1], MAX_FILE_NAME_SIZE -1) == 0){
       perror(RED"delete_entry():The given directory has children components, CANNOT BE DELETED.");
       return -4;
      }
    }
  }

  memset(de->name, 0, MAX_FILE_NAME_SIZE);
  strncpy(de->parentDir, ROOT_DIR, MAX_FILE_NAME_SIZE);
  de->size = 0;
  de->firstBlock = -1;
  de->isDir = -1;
  de->isTaken = 0;

  return 0;
}

int get_first_block(const char **names,struct directory_entry *dirTable) {
  struct directory_entry *de;
  if (!names || names[0]) {
    perror(RED"get_first_block(): Name is NULL\n");
    return -1;
  }
  if ((de = search_entry(names,0, dirTable)) == NULL) {  //no block assigned to a directory
    perror(RED"get_first_block(): Entry does not exist");
    return -2;
  }
  return de->firstBlock;
}

