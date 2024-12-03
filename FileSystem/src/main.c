
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include "../include/DirectoryTable.h"
#include "../include/FAT.h"
#include "../include/disk.h"
#include "../include/global.h"
#include "../include/hashmap.h"
#include "../include/operations.h"
#include "../include/parse.h"
#include "../include/uthash.h"
#include "../include/utils.h"

int numFreeBlocks;
int freeBlocksHead;
int numBlocksReserved;

unsigned char key[32] = {0x4f, 0x8d, 0x7f, 0x2e, 0x8c, 0x93, 0xd6, 0xfb,
                         0x53, 0xf1, 0xed, 0x2b, 0x86, 0xc8, 0xbf, 0xb1,
                         0x0c, 0x8e, 0xd8, 0x97, 0x9a, 0x7b, 0xe1, 0x1a,
                         0x08, 0xcd, 0x9d, 0x39, 0xc4, 0x3f, 0x21, 0x2b};

struct directory_entry dirTable[DIR_ENTERIES];
struct fat_entry fat[MAX_BLOCKS];
struct HashMap *map;

// we will use param here if we do the bonus parts
int initialze() {
  numBlocksReserved = num_metadata_blks_req();
  numFreeBlocks =
      MAX_BLOCKS - numBlocksReserved; // leave space for reserved blocks
  freeBlocksHead = numBlocksReserved;
  if (init_dir_table(dirTable) < 0) {
    perror(RED "initialize(): can not init dirTable");
    return -1;
  }
  if (init_fat(fat, &numFreeBlocks, &freeBlocksHead, numBlocksReserved) < 0) {
    perror(RED "initialize(): can not init fat");
    return -2;
  }
  map = NULL;
  return 0;
}

void build_path(struct directory_entry *dirTable, int currentIdx, char **pathBuffer) {
    if (dirTable[currentIdx].parentIdx == DEFAULT_PARENT) {
        snprintf(*pathBuffer, MAX_FILE_NAME_SIZE, "%s", dirTable[currentIdx].name);
        return;
    }

    char *temp = malloc(MAX_FILE_NAME_SIZE);
    if (temp == NULL) {
        perror("Memory allocation failed");
        exit(EXIT_FAILURE);
    }

    build_path(dirTable, dirTable[currentIdx].parentIdx, &temp);

    size_t newSize = strlen(temp) + strlen(dirTable[currentIdx].name) + 2;
    *pathBuffer = realloc(*pathBuffer, newSize);

    if (*pathBuffer == NULL) {
        perror("Memory allocation failed");
        exit(EXIT_FAILURE);
    }

    snprintf(*pathBuffer, newSize, "%s/%s", temp, dirTable[currentIdx].name);

    free(temp);
}

void construct_map(struct HashMap **map, struct directory_entry *dirTable, int tableSize) {
    for (int i = 0; i < tableSize; i++) {
        if (!dirTable[i].isTaken) continue; 

        char *fullPath = malloc(MAX_FILE_NAME_SIZE);
        if (fullPath == NULL) {
            perror("Memory allocation failed");
            exit(EXIT_FAILURE);
        }

        memset(fullPath, 0, MAX_FILE_NAME_SIZE);

        build_path(dirTable, i, &fullPath);

        if (dirTable[i].isDir == 1) {
            size_t newSize = strlen(fullPath) + 2;
            fullPath = realloc(fullPath, newSize);
            if (fullPath == NULL) {
                perror("Memory allocation failed");
                exit(EXIT_FAILURE);
            }
            strcat(fullPath, "/");
        }

        add_to_map(map, fullPath, i);

        free(fullPath);
    }
}

int serialize() {
  ssize_t bytesWritten = 0;
  int disk;
  char status = 'w';
  if (MAX_BLOCKS - numFreeBlocks < numBlocksReserved) {
    perror(RED "serialize(): not enough space" RESET);
    return -1;
  }
  int currBlock = 0;
  write_to_disk(&status, sizeof(char), (currBlock * BLOCK_SIZE) + bytesWritten);
  bytesWritten += sizeof(char);
  write_to_disk(&numFreeBlocks, sizeof(int),
                (currBlock * BLOCK_SIZE) + bytesWritten);
  bytesWritten += sizeof(int);
  write_to_disk(&freeBlocksHead, sizeof(int),
                (currBlock * BLOCK_SIZE) + bytesWritten);
  bytesWritten += sizeof(int);
  write_to_disk(&numBlocksReserved, sizeof(int),
                (currBlock * BLOCK_SIZE) + bytesWritten);
  bytesWritten += sizeof(int);

  if ((disk = open(DISK_NAME, O_RDWR, O_RDWR)) < 0) {
    perror(RED "serialze(): could not open disk" RESET);
    return -1;
  }
  size_t gdtBytes = save_dir_table(dirTable, disk, bytesWritten);
  if (gdtBytes <= 0) {
    perror(RED "Error writing dirTable" RESET);
    return -2;
  }
  bytesWritten += gdtBytes;

  if (bytesWritten > BLOCK_SIZE * numBlocksReserved) {
    perror(RED "serialize():Exceeded the reserved blocks" RESET);
    close(disk);
    return -3;
  }

  size_t fatBytes = save_fat(fat, disk, bytesWritten);

  if (fatBytes <= 0) {
    perror(RED "Error writing fat" RESET);
    return -2;
  }

  bytesWritten += fatBytes;

  if (bytesWritten > BLOCK_SIZE * numBlocksReserved) {
    perror(RED "serialize():Exceeded the reserved blocks" RESET);
    close(disk);
    return -3;
  }

  // ssize_t mapBytes = save_map(map, disk, bytesWritten);
  // if (mapBytes <= 0) {
  //   perror(RED "Error writing map" RESET);
  //   return -2;
  // }
  // bytesWritten += mapBytes;

  // if (bytesWritten > BLOCK_SIZE * numBlocksReserved) {
  //   perror(RED "serialize():Exceeded the reserved blocks" RESET);
  //   close(disk);
  //   return -3;
  // }

  close(disk);
  return 0;
}

int deserialize() {
  ssize_t bytesRead = 0;
  int disk;
  char st;
  if ((disk = open(DISK_NAME, O_RDONLY)) < 0) {
    perror(RED "deserialize(): could not open disk" RESET);
    return -1;
  }

  off_t offset = 0;
  bytesRead = read_from_disk(&st, sizeof(char), offset);
  offset += sizeof(char);
  if (st == 'w') {
    bytesRead = read_from_disk(&numFreeBlocks, sizeof(int), offset);
    if (bytesRead < sizeof(int)) {
      perror(RED "deserialize(): failed to read numFreeBlocks" RESET);
      close(disk);
      return -1;
    }
    offset += sizeof(int);

    bytesRead = read_from_disk(&freeBlocksHead, sizeof(int), offset);
    if (bytesRead < sizeof(int)) {
      perror(RED "deserialize(): failed to read freeBlocksHead" RESET);
      close(disk);
      return -1;
    }
    offset += sizeof(int);

    bytesRead = read_from_disk(&numBlocksReserved, sizeof(int), offset);
    if (bytesRead < sizeof(int)) {
      perror(RED "deserialize(): failed to read numBlocksReserved" RESET);
      close(disk);
      return -1;
    }
    offset += sizeof(int);
    if (load_dir_table(dirTable, disk, offset) < 0) {
      perror(RED "deserialize(): failed to load directory table" RESET);
      close(disk);
      return -1;
    }
    offset += sizeof(struct directory_entry) * DIR_ENTERIES;
    printDirectoryTable(dirTable);

    if (load_fat(fat, disk, offset) < 0) {
      perror(RED "deserialize(): failed to load FAT" RESET);
      close(disk);
      return -1;
    }

    // printFatTable(fat);
    offset += sizeof(struct fat_entry) * MAX_BLOCKS;
    construct_map(&map, dirTable, DIR_ENTERIES);
    // if (load_map(&map, disk, offset) < 0) {
    //   perror(RED "deserialize(): failed to load map" RESET);
    //   close(disk);
    //   return -1;
    // }
    print_map(map);
    close(disk);
  }
  return 0;
}

// int clear_file(const char *path, size_t size, off_t offset) {
//   int fd;
//   char *zeroBuffer;
//   ssize_t bytesWritten = 0;
//   fd = open(path, O_RDWR);
//   if (fd < 0) {
//     perror(RED "clear_file(): Failed to open file" RESET);
//     return -1;
//   }
//   zeroBuffer = (char *)calloc(size, sizeof(char));
//   if (!zeroBuffer) {
//     perror(RED "clear_file(): Memory allocation failed" RESET);
//     close(fd);
//     return -1;
//   }
//   bytesWritten = pwrite(fd, zeroBuffer, size, offset);
//   if (bytesWritten < 0) {
//     perror(RED "clear_file(): Write failed" RESET);
//     free(zeroBuffer);
//     close(fd);
//     return -1;
//   }
//   if ((size_t)bytesWritten != size) {
//     fprintf(stderr, RED "clear_file(): Incomplete clear operation" RESET);
//     free(zeroBuffer);
//     close(fd);
//     return -1;
//   }
//   free(zeroBuffer);
//   close(fd);
//   printf(GREEN "File cleared successfully\n" RESET);
//   return 0;
// }

// TODO: FREE tokenizedinput
int main() {
  char *line = NULL;
  size_t len = 0;
  int numEnteries = 0;
  size_t allocatedSize = 10;
  char **temp = NULL;
  if (initialze() < 0) {
    perror(RED "initialze()");
    return -1;
  }
  // ssize_t size = 64*1024*1024;
  // clear_file(DISK_NAME,size ,0);
  if (deserialize() < 0) {
    perror("deserialze()");
    return -2;
  }
  while (1) {
    char *tokenizedInput[allocatedSize];
    printf(YELLOW "cmd > " GREEN);
    if (getline(&line, &len, stdin) == -1) {
      perror(RED "getline(): Error reading input" RESET);
    }
    size_t length = strlen(line);
    if (length > 0 && line[length - 1] == '\n') {
      line[length - 1] = '\0';
    }
    if ((numEnteries = tokenize_input(line, tokenizedInput, &allocatedSize)) <=
        0) {
      perror(RED "Could not tokenize" RESET);
    }
    // free(line);
    if (numEnteries > 0) {
      if (strcmp(tokenizedInput[0], "quit") == 0) {
        break;
      }
      if (strcmp(tokenizedInput[0], MAKE_DIR) == 0) {
        // make directory
        temp = tokenizedInput + 1;
        create_directory_or_file(temp, numEnteries - 1, 0, 1, dirTable, fat,
                                 &numFreeBlocks, &freeBlocksHead, &map);

      } else if (strcmp(tokenizedInput[0], MAKE_FILE) == 0) {
        // make file
        temp = tokenizedInput + 1;
        create_directory_or_file(temp, numEnteries - 1, 0, 0, dirTable, fat,
                                 &numFreeBlocks, &freeBlocksHead, &map);
      } else if (strcmp(tokenizedInput[0], DELETE_DIR) == 0) {
        // delete directory
        temp = tokenizedInput + 1;
        delete_directory_or_file(temp, numEnteries - 1, 1, dirTable, fat,
                                 &numFreeBlocks, &freeBlocksHead, &map,
                                 numBlocksReserved);

      } else if (strcmp(tokenizedInput[0], DELETE_FILE) == 0) {
        // delete file
        temp = tokenizedInput + 1;
        delete_directory_or_file(temp, numEnteries - 1, 0, dirTable, fat,
                                 &numFreeBlocks, &freeBlocksHead, &map,
                                 numBlocksReserved);

      } else if (strcmp(tokenizedInput[0], LIST) == 0) {
        // list
        temp = tokenizedInput + 1;
        list_directory(temp, numEnteries - 1, dirTable, &map);
      } else if (strcmp(tokenizedInput[0], READ_FILE) == 0) {
        // read filek
        temp = tokenizedInput + 1;
        char *readBuf = NULL;
        size_t bytes = 0;
        off_t offset = 0;
        char *input = NULL;
        size_t le;
        printf(YELLOW "Enter the offset\n" GREEN);
        if (getline(&input, &le, stdin) != -1) {
          offset = strtoull(input, NULL, 10);
        }
        printf(YELLOW "Enter the number of bytes\n" GREEN);
        if (getline(&input, &le, stdin) != -1) {
          bytes = strtoull(input, NULL, 10);
        }
        free(line);
        read_from_file_offset(temp, numEnteries - 1, &readBuf, 0, dirTable, fat,
                              &numFreeBlocks, &freeBlocksHead, &map, offset,
                              bytes);
        printf("Data read: %s\n", readBuf);
      } else if (strcmp(tokenizedInput[0], WRITE_FILE) == 0) {
        // write file
        temp = tokenizedInput + 1;
        write_to_file(temp, numEnteries - 1, 0, dirTable, fat, &numFreeBlocks,
                      &freeBlocksHead, &map, numBlocksReserved);

      } else if (strcmp(tokenizedInput[0], TRUNCATE_FILE) == 0) {
        // truncate file
        temp = tokenizedInput + 1;
        size_t numBytes = 0;
        char *input = NULL;
        size_t le = 0;
        printf(GREEN "Enter the number of bytes you want to truncate from end "
                     "of the file\n" RESET);
        if (getline(&input, &le, stdin) != -1) {
          numBytes = strtoull(input, NULL, 10); // Convert input to size_t
        }
        free(input);

        truncate_file(temp, numEnteries - 1, numBytes, 0, &numFreeBlocks,
                      &freeBlocksHead, dirTable, fat, &map, numBlocksReserved);
      } else if (strcmp(tokenizedInput[0], "hello") == 0) {
      } else {
        printf(YELLOW "cmd > ");
        printf(GREEN "Invalid Command\n" RESET);
      }
      // printDirectoryTable(dirTable);
      // printFatTable(fat);
      // print_map(map);
    }
  }

  if (serialize() < 0) {
    perror("serialize()");
  }

  return 0;
}