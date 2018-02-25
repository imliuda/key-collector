#ifndef __OSCLT_RBTREE_H__
#define __OSCLT_RBTREE_H__

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>

#include "list.h"

enum color {RED, BLACK};

struct map_entry {
    void *key;
    void *data;
    struct map_entry *parent;
    struct map_entry *left;
    struct map_entry *right;
    enum color color;
};

struct map {
    struct map_entry *entries;
    int (*keycmp)(void *key1, void *key2);
};

struct map *map_new(int (*keycmp)(void *key1, void *key2));
bool map_has(struct map *map, void *key);
void map_add(struct map *map, void *key, void *data);
void map_update(struct map *map, void *key, void *data, void **orig_data);
void map_replace(struct map *map, void *key, void *data, void **orig_key, void **orig_data);
void map_remove(struct map *map, void *key, void **orig_key, void **orig_data);
void map_get(struct map *map, void *key, void **data);
void map_keys(struct map *map, struct list **keys);
void map_destroy(struct map *map);

#endif
