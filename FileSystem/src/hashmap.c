#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "../include/hashmap.h"
#include "../include/utils.h"
#include "../include/DirectoryTable.h"

void add_to_map(struct HashMap **map, const char *key, int value) {
  struct HashMap *entry = malloc(sizeof(struct HashMap));
  entry->key = strdup(key); // Allocate memory and copy key
  entry->value = value;
  HASH_ADD_KEYPTR(hh, *map, entry->key, strlen(entry->key),
                  entry); // Add to map
}

struct HashMap *find_in_map(struct HashMap *map, const char *key) {
  struct HashMap *entry;
  HASH_FIND_STR(map, key, entry); // Find the key
  return entry;
}

void delete_from_map(struct HashMap **map, const char *key) {
  struct HashMap *entry = find_in_map(*map, key);
  if (entry) {
    HASH_DEL(*map, entry); // Remove from hashmap
    free(entry->key);      // Free the allocated key memory
    free(entry);           // Free the entry
  }
}

void free_map(struct HashMap *map) {
  struct HashMap *current, *tmp;
  HASH_ITER(hh, map, current, tmp) {
    HASH_DEL(map, current); // Remove from hashmap
    free(current->key);     // Free the allocated key memory
    free(current);          // Free the entry
  }
}

int get_value(struct HashMap *map, const char *key) {
  struct HashMap *entry;
  HASH_FIND_STR(map, key, entry); // Search for the key in the hashmap
  if (entry) {
    return entry->value; // Return the value if found
  }
  return -1; // Return -1 if the key does not exist
}

void print_map(struct HashMap *map) {
  if (map == NULL) {
    printf("The map is empty.\n");
    return;
  }

  struct HashMap *entry;
  printf("Contents of the map:\n");
  printf("---------------------\n");
  for (entry = map; entry != NULL; entry = entry->hh.next) {
    printf("Key: %s, Value: %d\n", entry->key, entry->value);
  }
  printf("---------------------\n");
}

size_t get_map_size(const struct HashMap *map) {
  size_t size = 0;
  if (map == NULL) {
    return size;
  }
  const struct HashMap *entry, *tmp;
  HASH_ITER(hh, map, entry, tmp) {
    size += entry->hh.keylen;
    size += sizeof(unsigned int);
    size += sizeof(int);
  }
  return size + sizeof(unsigned int);  //for the total entries in map
}

unsigned int get_num_enteries_in_map(const struct HashMap *map) {
  unsigned int count = 0;
  if (map == NULL) {
    return 0;
  }
  const struct HashMap *entry, *tmp;
  HASH_ITER(hh, map, entry, tmp) { count++; }
  return count;
}



ssize_t save_map(struct HashMap *map, int fd, off_t offset) {
  ssize_t bytesWritten = 0;
  ssize_t totalBytesWritten = 0; 
  if (!map) {
    perror(RED "save_map(): map is null" RESET);
    return -1;
  }
  unsigned int numEntries = get_num_enteries_in_map(map);
  bytesWritten = pwrite(fd, &numEntries, sizeof(unsigned int), offset);
  if (bytesWritten < 0) {
    perror(RED "save_map(): write failed(num entries)" RESET);
    return -1;
  }
  if (bytesWritten < sizeof(unsigned int)) {
    perror(RED "save_map(): incomplete write(num entries)" RESET);
    return -1;
  }

  totalBytesWritten += bytesWritten;
  offset += bytesWritten;
  struct HashMap *entry;
  for (entry = map; entry != NULL; entry = entry->hh.next) {
    // Write the size of the key
    bytesWritten = pwrite(fd, &entry->hh.keylen, sizeof(unsigned int), offset);
    if (bytesWritten < 0) {
      perror(RED "save_map(): write failed(key size)" RESET);
      return -1;
    }
    if (bytesWritten < sizeof(unsigned int)) {
      perror(RED "save_map(): incomplete write(key size)" RESET);
      return -1;
    }

    totalBytesWritten += bytesWritten; 
    offset += bytesWritten;

    // Write the key
    bytesWritten = pwrite(fd, entry->key, (size_t)entry->hh.keylen, offset);
    if (bytesWritten < 0) {
      perror(RED "save_map(): write failed(key)" RESET);
      return -1;
    }
    if (bytesWritten < (size_t)entry->hh.keylen) {
      perror(RED "save_map(): incomplete write(key)" RESET);
      return -1;
    }

    totalBytesWritten += bytesWritten; // Update total bytes written
    offset += bytesWritten;

    // Write the value
    bytesWritten = pwrite(fd, &entry->value, sizeof(int), offset);
    if (bytesWritten < 0) {
      perror(RED "save_map(): write failed(value)" RESET);
      return -1;
    }
    if (bytesWritten < sizeof(int)) {
      perror(RED "save_map(): incomplete write(value)" RESET);
      return -1;
    }

    totalBytesWritten += bytesWritten; // Update total bytes written
    offset += bytesWritten;
  }

  return totalBytesWritten; // Return the total bytes written
}


int load_map(struct HashMap **map, int fd, off_t offset) {
  ssize_t bytesRead = 0;
  unsigned int numEntries = 0;
  unsigned int keyLen = 0;
  char *key = NULL;
  int value = 0;

  bytesRead = pread(fd, &numEntries, sizeof(unsigned int), offset);
  if (bytesRead < 0) {
    perror(RED "load_map(): read failed(num entries)" RESET);
    return -1;
  }
  if (bytesRead < sizeof(unsigned int)) {
    perror(RED "load_map(): incomplete read(num entries)" RESET);
    return -1;
  }

  offset += sizeof(unsigned int);

  for (unsigned int i = 0; i < numEntries; i++) {
    // Read the length of the key
    bytesRead = pread(fd, &keyLen, sizeof(unsigned int), offset);
    if (bytesRead < 0) {
      perror(RED "load_map(): read failed(key size)" RESET);
      return -1;
    }
    if (bytesRead < sizeof(unsigned int)) {
      perror(RED "load_map(): incomplete read(key size)" RESET);
      return -1;
    }

    offset += sizeof(unsigned int);

    // Read the key
    key = realloc(key, keyLen + 1); // Resize key buffer
    if (!key) {
      perror(RED "load_map(): memory allocation for key failed" RESET);
      return -1;
    }

    bytesRead = pread(fd, key, keyLen, offset);
    if (bytesRead < 0) {
      perror(RED "load_map(): read failed(key)" RESET);
      return -1;
    }
    if ((size_t)bytesRead < keyLen) {
      perror(RED "load_map(): incomplete read(key)" RESET);
      return -1;
    }
    key[keyLen] = '\0'; // Null-terminate the key

    offset += keyLen;

    // Read the value
    bytesRead = pread(fd, &value, sizeof(int), offset);
    if (bytesRead < 0) {
      perror(RED "load_map(): read failed(value)" RESET);
      return -1;
    }
    if (bytesRead < sizeof(int)) {
      perror(RED "load_map(): incomplete read(value)" RESET);
      return -1;
    }

    offset += sizeof(int);

    add_to_map(map, key, value);
  }

  free(key);

  return 0;
}
