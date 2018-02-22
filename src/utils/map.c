#include "map.h"
#include <stdio.h>

/*
 * create a new rbtree.
 * 
 * same with "struct rbtree *tree = NULL;"
 *
 * @return a NULL pointer.
 */
struct map *map_new(int (*keycmp)(void *key1, void *key2)) {
    struct map *map = malloc(sizeof(struct map));
    map->entries = list_new();
    map->keycmp = keycmp;
    return map;
}

/*
 * test if map contains the key.
 */
bool map_has(struct map *map, void *key) {
    struct list *p = map->entries;
    struct map_entry *e;
    while (p = list_next(map->entries, p)) {
        e = (struct map_entry *)(p->data);
        if (map->keycmp(e->key, key) == 0) {
            return true;
        }
    }
    return false;
}

/*
 * add key value pairs to map.
 *
 * before calling this function, must call map_has() to check if key
 * already exisits.
 */
void map_add(struct map *map, void *key, void *data) {
    struct map_entry *e = malloc(sizeof(struct map_entry));
    struct list *node = malloc(sizeof(struct list));
    e->key = key;
    e->data = data;
    node->data = e;
    list_append(map->entries, node);
}

/*
 * update the data of an exists key.
 *
 * before calling this function, must call map_has() to check if key
 * already exisits.
 *
 * @param[output] if not NULL, stores the orig_data.
 */
void map_update(struct map *map, void *key, void *data, void **orig_data) {
    struct list *p = map->entries;
    struct map_entry *e;
    while (p = list_next(map->entries, p)) {
        e = (struct map_entry *)(p->data);
        if (map->keycmp(e->key, key) == 0) {
            if (orig_data != NULL) {
                *orig_data = e->data;
            }
            e->data = data;
            return;
        }
    }
}

/*
 * replace the key and the data of an exists entry.
 *
 * before calling this function, must call map_has() to check if key
 * already exisits. orig_key and orig_data use as storing the origin key and data.
 *
 * @param[putput] orig_key if not NULL, stores the origin key.
 * @param[output] orig_data if not NULL, stores the origin key's data.
 */
void map_replace(struct map *map, void *key, void *data, void **orig_key, void **orig_data) {
    struct list *p = map->entries;
    struct map_entry *e;
    while (p = list_next(map->entries, p)) {
        e = (struct map_entry *)(p->data);
        if (map->keycmp(e->key, key) == 0) {
            if (orig_key != NULL) {
                *orig_key = e->key;
            }
            if (orig_data != NULL) {
                *orig_data = e->data;
            }
            e->key = key;
            e->data = data;
            return;
        }
    }

}

/*
 * delete key value pair in map.
 *
 * before calling this function, must call map_has() to check if key
 * already exisits.
 */
void map_remove(struct map *map, void *key) {
    struct list *p = map->entries;
    struct map_entry *e;
    while (p = list_next(map->entries, p)) {
        e = (struct map_entry *)(p->data);
        if (map->keycmp(e->key, key) == 0) {
            list_remove(map->entries, p);
            return;
        }
    }
}

/*
 * get data for key.
 *
 * before calling this function, must call map_has() to check if key
 * already exisits.
 * 
 * @param map   the map to search
 * @param key   the key to lookup.
 * @param[output] data  store the found data, if data is NULL, this
 *              function will test if key is in map.
 */
void map_get(struct map *map, void *key, void **data) {
    struct list *p = map->entries;
    struct map_entry *e;
    while (p = list_next(map->entries, p)) {
        e = (struct map_entry *)(p->data);
        if (map->keycmp(e->key, key) == 0) {
            *data = e->data;
            return;
        }
    }
}

/*
 * get map keys which has been sorted.
 *
 * @param[output] list stores the keys, must be freed by user after use.
 */
void map_keys(struct map *map, struct list **keys) {
    struct list *p = map->entries;
    struct map_entry *e;
    *keys = list_new();
    while (p = list_next(map->entries, p)) {
        e = (struct map_entry *)(p->data);
        struct list *node = malloc(sizeof(struct list));
        node->data = e->key;
        list_append(*keys, node);
    }
    list_sort(*keys, map->keycmp);
}

/*
 * destroy a map. map data must be first freed by user.
 */
void map_destroy(struct map *map) {
    struct list *p = map->entries;
    while (p = list_next(map->entries, p)) {
        free(p->data);
    }
    list_destroy(map->entries);
}
