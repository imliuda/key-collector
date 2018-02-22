#include <stdio.h>
#include <string.h>

#include "../src/utils/map.h"
#include "../src/utils/list.h"

int strkeycmp(void *key1, void *key2) {
    return strcmp((const char *)key1, (const char *)key2);
}

void print_map(struct map *map) {
    struct list *keys;
    map_keys(map, &keys);
    printf("map:\n");
    struct list *p = keys;
    while (p = list_next(keys, p)) {
        int *value;
        map_get(map, p->data, (void *)&value);
        printf("    %s: %d\n", p->data, *value);
    }
    printf("\n");
}

int main () {
    char *key1 = "apple", *key2 = "orange", *key3 = "banana";
    int value1 = 1, value2 = 2, value3 = 3;
    struct map *map = map_new(strkeycmp);
    // map_add
    map_add(map, key2, &value2);
    map_add(map, key1, &value1);
    map_add(map, key3, &value3);
    // map_has
    printf("has apple: %d, orange: %d, banana: %d, peach: %d\n",
           map_has(map, "apple"), map_has(map, key2), map_has(map, key3), map_has(map, "peach"));
    // keys, get
    print_map(map);
    // update, repalce, remove
    char *orig_key;
    int *orig_data;
    int a1 = 7, a2 = 8;
    map_remove(map, "apple");
    map_update(map, "orange", &a1, (void **)&orig_data);
    printf("update orange: %d\n", *orig_data);
    map_replace(map, "banana", &a2, (void **)&orig_key, (void **)&orig_data);
    printf("replace banana: %s, %d\n", orig_key, *orig_data);
    print_map(map);
}
