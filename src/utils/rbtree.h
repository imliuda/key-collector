#ifndef __OSCLT_RBTREE_H__
#define __OSCLT_RBTREE_H__

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>

enum color {RED, BLACK};

struct rbnode {
    void *key;
    void *data;
    struct rbnode *parent;
    struct rbnode *left;
    struct rbnode *right;
    enum color color;
};

struct rbtree {
    struct rbnode *root;
    int (*keycmp)(void *key1, void *key2);
    void (*destroy)(void *key, void *data);
};

struct rbtree *rbtree_new(int keycmp(void *key1, void *key2),
                          void (*destroy)(void *key, void *data));
void rbtree_insert(struct rbtree *tree, void *key, void *data);
struct rbtree *rbtree_delete(struct rbtree *tree, void *key);
bool rbtree_has(struct rbtree *tree, void *key);
struct rbtree *rbtree_get(struct rbtree *tree, void *key);
void rbtree_update(struct rbtree *tree, void *key, void *data);
struct slist *rbtree_keys(struct rbtree *tree);
void rbtree_destroy(struct rbtree *tree);

#endif
