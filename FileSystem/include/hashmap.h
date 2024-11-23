#ifndef HASHMAP_H
#define HASHMAP_H

#include "uthash.h"

struct HashMap{
  char* key;
  int value; 
  UT_hash_handle hh;
};

void add_to_map(struct HashMap **map, const char *key, int value);
struct HashMap *find_in_map(struct HashMap *map, const char *key);
void delete_from_map(struct HashMap **map, const char *key);
void free_map(struct HashMap *map);
int get_value(struct HashMap *map, const char *key);
void print_map(struct HashMap *map);
#endif