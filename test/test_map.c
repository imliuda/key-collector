#include <stdio.h>
#include <string.h>

#include "../src/utils/map.h"
#include "../src/utils/list.h"

int strkeycmp(void *key1, void *key2) {
    return strcmp((const char *)key1, (const char *)key2);
}

int main () {
    struct map *map = map_new(strkeycmp);
    char *s = malloc(20);
    strcpy(s, "test");
    map_add(map, s, s);
    map_add(map, "aaa", "bbb");
    struct list *p, *list = map->entries;
    struct map_entry *e;
    p = list;
    while (p = list_next(list, p)) {
        e = (struct map_entry *)(p->data);
        printf("key: %s, value: %s\n", e->key, e->data);
    }
    char *data;
    map_get(map, "aaa", (void *)&data);
    printf("%s\n", data);
}
