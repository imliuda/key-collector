#include <stdio.h>
#include <string.h>
#include <time.h>

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
        char *value;
        map_get(map, p->data, (void *)&value);
        printf("    %s: %s, (%ld: %ld)\n", p->data, value, p->data, value);
    }
    printf("\n");
}

char *random_str(int max) {
    int length = random() % 5 + 1;
    char *s = malloc(sizeof(length + 1));
    for (int i = 0; i < length; i++) {
        s[i] = random() % 26 + 'a';
    }
    s[length] = '\0';
    return s;
}

int main () {
    char *orig_key, *orig_data, *data;
    struct list *list = list_new();
    struct map *map = map_new(strkeycmp);

    srandom(time(NULL));
    for (int i = 0; i < 7; i++) {
        char *s = random_str(5);
        list_append(list, list_node(s));
        printf("add: %s\n", s);
        map_add(map, s, s);
    }
    print_map(map);
    char *s1 = "1", *s2 = "2";
    map_update(map, list_first(list)->data, s1, (void **)&orig_data);
    printf("updated %s, orig_data: %ld\n", list_first(list)->data, orig_data);
    char *new_key = strdup(list_last(list)->data);
    map_replace(map, new_key, s2, (void **)&orig_key, (void **)&orig_data);
    printf("replaced %s, orig_key: %ld, orig_data: %ld\n", new_key, orig_key, orig_data);
    printf("has %s: %d, has NOWAY: %d\n", list_first(list)->data, map_has(map, list_first(list)->data), map_has(map, "NOWAY"));
    print_map(map);
    // remove
    printf("AFTER remove:\n");
    struct list *p = list;
    for (int i = 0; i < 3; i++) {
        p = list_next(list, p);
        printf("remove: %s\n", p->data);
        map_remove(map, p->data, (void *)&orig_key, (void *)&orig_data);
    }
    print_map(map);
}
