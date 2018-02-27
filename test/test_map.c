#include <stdio.h>
#include <string.h>
#include <time.h>

#include "../src/utils/map.h"
#include "../src/utils/list.h"

int strkeycmp(void *key1, void *key2) {
    return strcmp((const char *)key1, (const char *)key2);
}

struct print_node {
    void *key;
    int padding;
    int level;
};

void print_map_build(struct map_entry *entry, struct list *nodes, int level) {
    if (entry == NULL)
    return;

    struct print_node *node = malloc(sizeof(struct print_node));
    char red[] = " R";
    char black[] = " B";

    print_map_build(entry->left, nodes, level + 1);

    node->level = level;
    node->key = malloc(strlen(entry->key) + 3);
    memcpy(node->key, entry->key, strlen(entry->key));
    if (entry->color == BLACK)
        memcpy(node->key + strlen(entry->key), black, 3);
    else
    memcpy(node->key + strlen(entry->key), red, 3);
    list_append(nodes, list_node(node));

    print_map_build(entry->right, nodes, level + 1);
}

void print_map(struct map *map) {
    struct list *p, *nodes = list_new();
    struct print_node *n;
    int padding, level;

    print_map_build(map->entries, nodes, 0);

    padding = 0;
    p = nodes;
    while (p = list_next(nodes, p)) {
        n = p->data;
        n->padding = padding;
        padding += strlen(n->key);
    };

    level = 0;
    p = nodes;
    while (p = list_next(nodes, p)) {
        n = p->data;
        if (n->level > level)
                level = n->level;
        }
    
        for (int i = 0; i < level + 1; i++) {
            char *line = malloc(2048);
            for(int j = 0; j < 2048; j++) {
            line[j] = ' ';
        }
        padding = 0;
        p = nodes;
        while (p = list_next(nodes, p)) {
            n = p->data;
            if (n->level != i)
                continue;
            memcpy(&line[n->padding], n->key, strlen(n->key));
        }
        line[n->padding + strlen(n->key)] = '\0';
        printf(">> %s\n", line);
        free(line);
    }
}

int get_random() {
    int num;
    FILE *fp = fopen("/dev/urandom", "r");
    fread(&num, sizeof(int), 1, fp);
    fclose(fp);
    if (num < 0) {
        num *= -1;
    }
    return num;
}

char *random_str(int max) {
    int length = get_random() % 5 + 1;
    printf("random: %d\n", length);
    char *s = malloc(sizeof(length + 1));
    for (int i = 0; i < length; i++) {
        s[i] = get_random() % 26 + 'a';
    }
    s[length] = '\0';
    return s;
}

int main () {
    char *orig_key, *orig_data, *data;
    struct list *list = list_new();
    struct map *map = map_new(strkeycmp);

    srandom(time(NULL));
    for (int i = 0; i < 15; i++) {
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
    for (int i = 0; i < 7; i++) {
        p = list_next(list, p);
        printf("remove: %s\n", p->data);
        map_remove(map, p->data, (void *)&orig_key, (void *)&orig_data);
        print_map(map);
    }
}
