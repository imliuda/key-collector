#include <stdio.h>
#include <string.h>
#include <time.h>

#include "../src/map.h"
#include "../src/list.h"

int strkeycmp(void *key1, void *key2) {
    return strcmp((const char *)key1, (const char *)key2);
}

struct print_node {
    void *key;
    int padding;
    int level;
};

struct list *print_map_build(struct map_entry *entry, int level) {
    if (entry == NULL)
    return NULL;

    struct print_node *node = malloc(sizeof(struct print_node));
    char red[] = " R";
    char black[] = " B";

    struct list *nodes = print_map_build(entry->left, level + 1);

    node->level = level;
    node->key = malloc(strlen(entry->key) + 3);
    memcpy(node->key, entry->key, strlen(entry->key));
    if (entry->color == BLACK)
        memcpy(node->key + strlen(entry->key), black, 3);
    else
    memcpy(node->key + strlen(entry->key), red, 3);
    nodes = list_append(nodes, node);

    nodes = list_extend(nodes, print_map_build(entry->right, level + 1));
}

void print_map(struct map *map) {
    struct list *p;
    struct print_node *n;
    int padding, level;

    struct list *nodes = print_map_build(map->entries, 0);

    level = 0;
    p = nodes;
    while (p) {
        n = p->data;
        if (n->level > level) {
                level = n->level;
        }
        p = list_next(p);
    }

    for (int cl = 0; cl <= level; cl++) {
        padding = 0;
        for (p = nodes; p != NULL; p = list_next(p)) {
            n = p->data;
            n->padding = padding;
            padding += strlen(n->key);
        };

        char *line = malloc(2048);
        for(int j = 0; j < 2048; j++)
            line[j] = ' ';

        for(p = nodes; p != NULL; p = list_next(p)) {
            n = p->data;
            if (n->level != cl)
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

    for (int i = 0; i < 15; i++) {
        char *s = random_str(5);
        list = list_append(list, s);
        printf("add: %s, return %d\n", s, map_add(map, s, s));
    }
    print_map(map);
    char *s1 = "1", *s2 = "2";
    printf("get should true: %d\n", map_get(map, list_data(list), (void **)&data));
    printf("update should true: %d\n", map_update(map, list_data(list), s1, (void **)&orig_data));
    printf("updated %s, orig_data: %ld\n", list_data(list), orig_data);
    char *new_key = strdup(list_data(list));
    printf("replace should true: %d\n", map_replace(map, new_key, s2, (void **)&orig_key, (void **)&orig_data));
    printf("replaced %s, orig_key: %ld, orig_data: %ld\n", new_key, orig_key, orig_data);
    printf("has %s: %d, has NOWAY: %d\n", list_data(list), map_has(map, list_data(list)), map_has(map, "NOWAY"));
    print_map(map);
    // remove
    printf("AFTER remove:\n");
    struct list *p = list;
    for (int i = 0; i < 7; i++) {
        printf("remove: %s, return %d\n", p->data, map_remove(map, p->data, (void *)&orig_key, (void *)&orig_data));
        print_map(map);
        p = list_next(p);
    }
}
