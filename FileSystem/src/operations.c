#include <stdio.h>
#include <stdlib.h>
#include <openssl/aes.h>
#include <openssl/rand.h>


#include "../include/DirectoryTable.h"
#include "../include/FAT.h"
#include "../include/disk.h"
#include "../include/global.h"
#include "../include/hashmap.h"
#include "../include/parse.h"
#include "../include/uthash.h"
#include "../include/utils.h"

// AES encryption function
int aes_encrypt(const unsigned char *input, unsigned char *output, size_t input_len, const unsigned char *key) {
    AES_KEY encryptKey;
    if (AES_set_encrypt_key(key, 128, &encryptKey) < 0) {
        perror("aes_encrypt(): AES key setup failed");
        return -1;
    }

    int block_count = (input_len + AES_BLOCK_SIZE - 1) / AES_BLOCK_SIZE;
    unsigned char iv[AES_BLOCK_SIZE];
    if (!RAND_bytes(iv, AES_BLOCK_SIZE)) {
        perror("aes_encrypt(): Failed to generate random IV");
        return -2;
    }
    for (int i = 0; i < block_count; i++) {
        AES_cbc_encrypt(input + i * AES_BLOCK_SIZE, output + i * AES_BLOCK_SIZE, AES_BLOCK_SIZE, &encryptKey, iv, AES_ENCRYPT);
    }

    return 0;
}


int aes_decrypt(const unsigned char *input, unsigned char *output, size_t input_len, const unsigned char *key) {
    AES_KEY decryptKey;
    if (AES_set_decrypt_key(key, 128, &decryptKey) < 0) {
        perror("aes_decrypt(): AES key setup failed");
        return -1;
    }
    unsigned char iv[AES_BLOCK_SIZE]; 
    memset(iv, 0, AES_BLOCK_SIZE); 
    int block_count = input_len / AES_BLOCK_SIZE;
    for (int i = 0; i < block_count; i++) {
        AES_cbc_encrypt(input + i * AES_BLOCK_SIZE, output + i * AES_BLOCK_SIZE, AES_BLOCK_SIZE, &decryptKey, iv, AES_DECRYPT);
    }

    return 0;
}


int create_directory_or_file(char **temp, int numEnteries, size_t size,
                             int isDir, struct directory_entry *dirTable,
                             struct fat_entry *fat, int *numFreeBlocks,
                             int *freeBlocksHead, struct HashMap **map) {
  add_entry(temp, numEnteries, size, isDir, dirTable, fat, numFreeBlocks,
            freeBlocksHead, map);
  return 0;
}

int delete_directory_or_file(char **temp, int numEnteries, int isDir,
                             struct directory_entry *dirTable,
                             struct fat_entry *fat, int *numFreeBlocks,
                             int *freeBlocksHead, struct HashMap **map, int numReservedBlocks) {
  int idx = -1;
  int head_idx = -1;
  if ((idx = search_entry(temp, numEnteries, isDir, map)) < 0) {
    perror(RED "Directory does not exist" RESET);
    return -1;
  }
  if (idx > 0) {
    head_idx = dirTable[idx].firstBlock;
    free_fat_enteries(head_idx, fat, freeBlocksHead, numFreeBlocks, numReservedBlocks);
  }
  delete_entry(temp, numEnteries, isDir, dirTable, map);
  return 0;
}

size_t greater_min_smaller(size_t a, size_t b) {
  return (a > b) ? a - b : b - a;
}

int write_to_file(char **temp, int numEnteries, int isDir,
                  struct directory_entry *dirTable, struct fat_entry *fat,
                  int *numFreeBlocks, int *freeBlocksHead,
                  struct HashMap **map, int numReservedBlocks) {
  int idx = -1;
  char *writeBuf = NULL;
  size_t bytesToWrite = -1;
  size_t currSize = -1;
  int currBlock = -1;
  int head = -1;
  size_t off = 0;
  size_t diskOffset;
  size_t len = 0;
  if ((idx = search_entry(temp, numEnteries, isDir, map)) < 0) {
    perror(RED "write_to_file():The target file does not exits" RESET);
    return -1;
  }
  if (dirTable[idx].isDir) {
    perror(RED "write_to_file(): Target is a directory, cannot write" RESET);
    return -2;
  }
  currSize = dirTable[idx].size;
  printf(
      YELLOW
      "\nEnter the data you want to write(end with enter key press)\n" GREEN);
  if (getline(&writeBuf, &bytesToWrite, stdin) == -1) {
    perror(RED "write_to_file(): getline()" RESET);
    return -3;
  }
  len = strlen(writeBuf);
  if (currSize == 0) {
    if (can_accomodate_n_size(len, numFreeBlocks, fat) < 0) {
      perror(RED "write_to_file(): Entered data is too large, cannot be "
                 "accomodated" RESET);
      return -4;
    }
    free_fat_enteries(dirTable[idx].firstBlock, fat, freeBlocksHead,
                      numFreeBlocks,numReservedBlocks);
  } else if (currSize > 0) {

    if (currSize < len) {
      if (can_accomodate_n_size(len - currSize, numFreeBlocks, fat) < 0) {
        perror(RED "write_to_file(): Entered data is too large, cannot be "
                   "accomodated" RESET);
        return -5;
      }
    }
    currBlock = dirTable[idx].firstBlock;
    while (currSize > 0 && currBlock >= 0) {
      if (currSize < BLOCK_SIZE) {
        truncate_at_offset(currSize, currBlock * BLOCK_SIZE);
        currSize = currSize - currSize;
      } else if (currSize > BLOCK_SIZE) {
        truncate_at_offset(BLOCK_SIZE, currBlock * BLOCK_SIZE);
        currSize -= BLOCK_SIZE;
      }
      currBlock = fat[currBlock].next;
    }

    free_fat_enteries(dirTable[idx].firstBlock, fat, freeBlocksHead,
                      numFreeBlocks, numReservedBlocks);
  }
  if ((head = reserve_blocks_for_n_size(len, numFreeBlocks, freeBlocksHead,
                                        fat)) < 0) {
    perror(RED "write_to_file(): Could not reserve blocks");
    return -6;
  }
  dirTable[idx].size = len;
  dirTable[idx].firstBlock = head;

  while (off < len && head >= 0) {
    diskOffset = head * BLOCK_SIZE;
    size_t chunkSize = (len - off) < BLOCK_SIZE ? (len - off) : BLOCK_SIZE;
    char *chunk = (char *)malloc(chunkSize);
    if (chunk == NULL) {
      perror(RED "write_to_file(): chunk is null" RESET);
      return -7;
    }
    memcpy(chunk, writeBuf + off, chunkSize);
    write_to_disk(chunk, chunkSize, diskOffset);

    off += chunkSize;
    head = fat[head].next;
    free(chunk);
  }
  return 0;
}


int read_from_file_offset(char **temp, int numEntries, char **readBuf, int isDir,
                          struct directory_entry *dirTable, struct fat_entry *fat,
                          int *numFreeBlocks, int *freeBlocksHead,
                          struct HashMap **map, size_t offset, size_t bytesToRead) {
  int idx = -1;
  int head = -1;
  size_t fileSize = 0;
  size_t chunkSize = 0;
  char *chunk = NULL;
  size_t bytesRead = 0; 
  size_t totalBytesRead = 0; 
  
  // Find the directory entry
  if ((idx = search_entry(temp, numEntries, isDir, map)) < 0) {
    perror(RED "read_from_file_offset(): entry does not exist" RESET);
    return -1;
  }

  head = dirTable[idx].firstBlock;
  fileSize = dirTable[idx].size;

  // Validate offset and bytesToRead
  if (offset >= fileSize) {
    perror(RED "read_from_file_offset(): offset out of bounds" RESET);
    return -2;
  }
  if (offset + bytesToRead > fileSize) {
    bytesToRead = fileSize - offset;  // Adjust to read only up to EOF
  }

  // Allocate buffer
  if ((*readBuf = (char *)malloc(bytesToRead + 1)) == NULL) {
    perror(RED "read_from_file_offset(): malloc()" RESET);
    return -3;
  }
  memset(*readBuf, 0, bytesToRead + 1);

  // Locate starting block and adjust offset within the block
  size_t currentOffset = 0;
  while (currentOffset + BLOCK_SIZE <= offset && head != LAST_BLOCK) {
    currentOffset += BLOCK_SIZE;
    head = fat[head].next;
  }
  offset -= currentOffset;  // Offset within the block

  // Read data
  while (bytesToRead > 0 && head != LAST_BLOCK) {
    chunkSize = (bytesToRead > BLOCK_SIZE - offset) ? BLOCK_SIZE - offset : bytesToRead;
    if ((chunk = (char *)malloc(chunkSize)) == NULL) {
      perror(RED "read_from_file_offset(): malloc()" RESET);
      return -4;
    }

    // Read from disk
    if ((bytesRead = read_from_disk(chunk, chunkSize, head * BLOCK_SIZE + offset)) < 0) {
      perror(RED "read_from_file_offset(): could not read from disk" RESET);
      free(chunk);
      return -5;
    }

    // Append to the result buffer
    strncat(*readBuf, chunk, bytesRead);
    totalBytesRead += bytesRead;
    bytesToRead -= bytesRead;
    offset = 0;  // Reset offset for subsequent blocks
    head = fat[head].next;

    free(chunk);
  }

  return (totalBytesRead == 0) ? -6 : 0;  // Return 0 if successful, or error code
}



// int write_to_file(char **temp, int numEnteries, int isDir,
//                   struct directory_entry *dirTable, struct fat_entry *fat,
//                   int *numFreeBlocks, int *freeBlocksHead,
//                   struct HashMap **map, int numReservedBlocks, const unsigned char *key) {
//   int idx = -1;
//   char *writeBuf = NULL;
//   size_t bytesToWrite = -1;
//   size_t currSize = -1;
//   int currBlock = -1;
//   int head = -1;
//   size_t off = 0;
//   size_t diskOffset;
//   size_t len = 0;

//   if ((idx = search_entry(temp, numEnteries, isDir, map)) < 0) {
//     perror(RED "write_to_file():The target file does not exist" RESET);
//     return -1;
//   }
//   if (dirTable[idx].isDir) {
//     perror(RED "write_to_file(): Target is a directory, cannot write" RESET);
//     return -2;
//   }

//   currSize = dirTable[idx].size;
//   printf(YELLOW "\nEnter the data you want to write (end with enter key press)\n" GREEN);
//   if (getline(&writeBuf, &bytesToWrite, stdin) == -1) {
//     perror(RED "write_to_file(): getline()" RESET);
//     return -3;
//   }

//   len = strlen(writeBuf);

//   if (currSize == 0) {
//     if (can_accomodate_n_size(len, numFreeBlocks, fat) < 0) {
//       perror(RED "write_to_file(): Entered data is too large, cannot be accommodated" RESET);
//       return -4;
//     }
//     free_fat_enteries(dirTable[idx].firstBlock, fat, freeBlocksHead, numFreeBlocks, numReservedBlocks);
//   } else if (currSize > 0) {
//     if (currSize < len) {
//       if (can_accomodate_n_size(len - currSize, numFreeBlocks, fat) < 0) {
//         perror(RED "write_to_file(): Entered data is too large, cannot be accommodated" RESET);
//         return -5;
//       }
//     }
//     // Handling truncation and free blocks omitted for brevity
//     free_fat_enteries(dirTable[idx].firstBlock, fat, freeBlocksHead, numFreeBlocks, numReservedBlocks);
//   }

//   if ((head = reserve_blocks_for_n_size(len, numFreeBlocks, freeBlocksHead, fat)) < 0) {
//     perror(RED "write_to_file(): Could not reserve blocks");
//     return -6;
//   }

//   dirTable[idx].size = len;
//   dirTable[idx].firstBlock = head;

//   unsigned char *encryptedData = (unsigned char *)malloc(len);
//   if (encryptedData == NULL) {
//     perror(RED "write_to_file(): Memory allocation for encrypted data failed" RESET);
//     return -7;
//   }

//   // Encrypt data
//   if (aes_encrypt((unsigned char *)writeBuf, encryptedData, len, key) < 0) {
//     free(encryptedData);
//     return -8;
//   }

//   while (off < len && head >= 0) {
//     diskOffset = head * BLOCK_SIZE;
//     size_t chunkSize = (len - off) < BLOCK_SIZE ? (len - off) : BLOCK_SIZE;
//     unsigned char *chunk = encryptedData + off;
//     write_to_disk(chunk, chunkSize, diskOffset);

//     off += chunkSize;
//     head = fat[head].next;
//   }

//   free(encryptedData);
//   return 0;
// }


// int read_from_file_offset(char **temp, int numEntries, char **readBuf, int isDir,
//                           struct directory_entry *dirTable, struct fat_entry *fat,
//                           int *numFreeBlocks, int *freeBlocksHead,
//                           struct HashMap **map, size_t offset, size_t bytesToRead, const unsigned char *key) {
//   int idx = -1;
//   int head = -1;
//   size_t fileSize = 0;
//   size_t chunkSize = 0;
//   char *chunk = NULL;
//   size_t bytesRead = 0; 
//   size_t totalBytesRead = 0;

//   if ((idx = search_entry(temp, numEntries, isDir, map)) < 0) {
//     perror(RED "read_from_file_offset(): entry does not exist" RESET);
//     return -1;
//   }

//   head = dirTable[idx].firstBlock;
//   fileSize = dirTable[idx].size;
//   if (offset >= fileSize) {
//     perror(RED "read_from_file_offset(): offset out of bounds" RESET);
//     return -2;
//   }
//   if (offset + bytesToRead > fileSize) {
//     bytesToRead = fileSize - offset;  
//   }

 
//   if ((*readBuf = (char *)malloc(bytesToRead + 1)) == NULL) {
//     perror(RED "read_from_file_offset(): malloc()" RESET);
//     return -3;
//   }
//   memset(*readBuf, 0, bytesToRead + 1);


//   size_t currentOffset = 0;
//   while (currentOffset + BLOCK_SIZE <= offset && head != LAST_BLOCK) {
//     currentOffset += BLOCK_SIZE;
//     head = fat[head].next;
//   }
//   offset -= currentOffset;  

//   unsigned char *decryptedData = (unsigned char *)malloc(bytesToRead);
//   if (decryptedData == NULL) {
//     perror(RED "read_from_file_offset(): Memory allocation for decrypted data failed" RESET);
//     return -4;
//   }
//   while (bytesToRead > 0 && head != LAST_BLOCK) {
//     chunkSize = (bytesToRead > BLOCK_SIZE - offset) ? BLOCK_SIZE - offset : bytesToRead;
//     if ((chunk = (char *)malloc(chunkSize)) == NULL) {
//       perror(RED "read_from_file_offset(): malloc()" RESET);
//       free(decryptedData);
//       return -5;
//     }
//     if ((bytesRead = read_from_disk(chunk, chunkSize, head * BLOCK_SIZE + offset)) < 0) {
//       free(chunk);
//       free(decryptedData);
//       return -6;
//     }
//     if (aes_decrypt((unsigned char *)chunk, decryptedData + totalBytesRead, bytesRead, key) < 0) {
//       free(chunk);
//       free(decryptedData);
//       return -7;
//     }

//     totalBytesRead += bytesRead;
//     bytesToRead -= bytesRead;
//     free(chunk);
//     head = fat[head].next;
//   }
//   memcpy(*readBuf, decryptedData, totalBytesRead);
//   free(decryptedData);

//   return totalBytesRead };



int truncate_file(char **temp, int numEnteries, size_t numBytes, int isDir,
                  int *numFreeBlocks, int *freeBlocksHead,
                  struct directory_entry *dirTable, struct fat_entry *fat,
                  struct HashMap **map, int numReservedBlocks) {
  int idx = -1;
  int head = -1;
  int size = 0;
  int *blockChain;
  int count = 0;
  size_t numB = numBytes;
  if ((idx = search_entry(temp, numEnteries, isDir, map)) < 0) {
    perror(RED "truncate_file(): entry does not exist" RESET);
    return -1;
  }
  head = dirTable[idx].firstBlock;
  size = dirTable[idx].size;
  if (numBytes > size || numBytes <= 0) {
    perror(RED "trncate_file(), invalid numBytes" RESET);
    return -2;
  }
  blockChain = get_chain_of_blocks(head, &count, fat);

  size_t bytesInLastBlock = size % BLOCK_SIZE;

  if (numBytes > BLOCK_SIZE) {
    truncate_at_offset(bytesInLastBlock,
                       blockChain[count - 1] *
                           BLOCK_SIZE); // remove all the bytes in last block
    numBytes -= bytesInLastBlock;
    fat[blockChain[count - 1]].isTaken = 0;
    fat[blockChain[count - 1]].next = BLOCK_EMPTY; // mark the block as free
    if (count - 2 >= 0) {
      fat[blockChain[count - 2]].next = LAST_BLOCK;
    } else {
      fat[head].next = LAST_BLOCK;
    }
    count--;
    while (numBytes > 0 && count - 1 >= 0) { // until all bytes are not removen
      if (numBytes >= BLOCK_SIZE) {
        truncate_at_offset(BLOCK_SIZE,
                           blockChain[count - 1] *
                               BLOCK_SIZE); // keep removing until they are
                                            // greater than BLOCK_SIZE
        numBytes -= BLOCK_SIZE;
        fat[blockChain[count - 1]].isTaken = 0;
        fat[blockChain[count - 1]].next = BLOCK_EMPTY;
        if (count - 2 >= 0) {
          fat[blockChain[count - 2]].next = LAST_BLOCK;
        } else {
          fat[head].next = LAST_BLOCK;
        }
        count--;
      } else { // else adjust the offset to remove accordingly
        truncate_at_offset(numBytes, blockChain[count - 1] * BLOCK_SIZE +
                                         (BLOCK_SIZE - numBytes));
        numBytes -= numBytes;
        count--;
      }
    }
  } else {
    if (bytesInLastBlock >= numBytes) {
      truncate_at_offset(numBytes, (blockChain[count - 1] * BLOCK_SIZE) +
                                       (bytesInLastBlock - numBytes));
      fat[blockChain[count - 1]].isTaken = 0;
      fat[blockChain[count - 1]].next = BLOCK_EMPTY;
      if (count - 2 >= 0) {
        fat[blockChain[count - 2]].next = LAST_BLOCK;
      } else {
        fat[head].next = LAST_BLOCK;
      }
    } else {
      truncate_at_offset(bytesInLastBlock,
                         blockChain[count - 1] *
                             BLOCK_SIZE); // remove all from last block
      truncate_at_offset(numBytes - bytesInLastBlock,
                         (blockChain[count - 2] * BLOCK_SIZE) +
                             (BLOCK_SIZE - (numBytes - bytesInLastBlock)));
      fat[blockChain[count - 1]].isTaken = 0;
      fat[blockChain[count - 1]].next = BLOCK_EMPTY;
      if (count - 2 >= 0) {
        fat[blockChain[count - 2]].next = LAST_BLOCK;
      } else {
        fat[head].next = LAST_BLOCK;
      }
    }
  }
  dirTable[idx].size = size - numB;
  if (dirTable[idx].size == 0) {
    fat[dirTable[idx].firstBlock].next = BLOCK_EMPTY;
    dirTable[idx].firstBlock = DEFAULT_FIRST_BLOCK;
  }
  make_free_chain(freeBlocksHead, numFreeBlocks, fat, numReservedBlocks);
  return 0;
}

int list_directory(char **temp, int numEnteries,
                   struct directory_entry *dirTable,
                   struct HashMap **map) {
  int idx = -1;
  char **children;
  int count = 0;
  if(strcmp(temp[0], ROOT_DIR) == 0){
    idx = DEFAULT_PARENT;
  }
  else if((idx = search_entry(temp, numEnteries, 1, map)) < 0) {
    perror(RED "list_directory(): the directory does not exits" RESET);
    return -1;
  }

  if ((children = malloc(sizeof(char *) * DIR_ENTERIES)) == NULL) {
    perror(RED "list_directory(): malloc()" RESET);
    return -2;
  }

  for (int i = 0; i < DIR_ENTERIES; i++) {
    if ((children[i] = malloc(MAX_FILE_NAME_SIZE * sizeof(char))) == NULL) {
      for (int j = 0; j < i; j++) {
        free(children[j]);
      }
      free(children);
      perror(RED "list_directory(): malloc()" RESET);
      return -3;
    }
  }

  if (get_children(idx, &children, &count, dirTable) < 0) {
    perror(RED "list_directory(): Error getting children" RESET);
    return -4;
  }

  char *path = construct_path(temp, numEnteries, 1);
  printDirectory(path, children, count, 0);
  free(path);
  for (int i = 0; i < DIR_ENTERIES; i++) {
    free(children[i]);
  }
  free(children);
  return 0; 
}