#include <stdlib.h>
#include <string.h>
#include <stdio.h>

#include "../include/hashmap.h"

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
