#include "rbtree.h"
#include <stdio.h>

/*
 * create a new rbtree.
 * 
 * same with "struct rbtree *tree = NULL;"
 *
 * @return a NULL pointer.
 */
struct rbtree *rbtree_new(int (*keycmp)(void *key1, void *key2),
                           void (*destroy)(void *key, void *data)) {
    struct rbtree *tree = malloc(sizeof(struct rbtree));
    tree->root = NULL;
    tree->keycmp = keycmp;
    tree->destroy = destroy;
    return tree;
}

static void rbtree_rotate_left(struct rbtree *tree, struct rbnode *node) {
    if (node->right->left) {
        node->right->left->parent = node->right->parent;
    }
    node->right->parent = node->parent;
    node->parent = node->right;
    node->right = node->parent->left;
    node->parent->left = node;

    if (node->parent->parent == NULL) {
        tree->root = node->parent;
    } else if (node == node->parent->parent->left) {
        node->parent->parent->left = node->parent;
    } else {
        node->parent->parent->right = node->parent;
    }
}

static void rbtree_rotate_right(struct rbtree *tree, struct rbnode *node) {
    if (node->left->right) {
        node->left->right->parent = node->left->parent;
    }
    node->left->parent = node->parent;
    node->parent = node->left;
    node->left = node->parent->right;
    node->parent->right = node;

    if (node->parent->parent == NULL) {
        tree->root = node->parent;
    } else if (node == node->parent->parent->left) {
        node->parent->parent->left = node->parent;
    } else {
        node->parent->parent->right = node->parent;
    }
}

static void rbtree_insert2(struct rbtree *tree, struct rbnode *node);

/*
 * when parent is RED, and uncle's color is RED.
 */
static void rbtree_insert3(struct rbtree *tree, struct rbnode *node) {
    struct rbnode *uncle = node->parent == node->parent->parent->left ? node->parent->parent->right : node->parent->parent->left;
    node->parent->color = BLACK;
    uncle->color = BLACK;
    node->parent->parent->color = RED;
    if (node->parent->parent->parent == NULL) {
        node->parent->parent->color = BLACK;
    } else if (node->parent->parent->parent->color == RED) {
        rbtree_insert2(tree, node->parent->parent);
    }
}

/*
 * when parent is RED, if uncle not exists, or uncle's color is BLACK.
 */
static void rbtree_insert2(struct rbtree *tree, struct rbnode *node) {
    struct rbnode *uncle = node->parent == node->parent->parent->left ? node->parent->parent->right : node->parent->parent->left;
    if (uncle == NULL || uncle->color == BLACK) {
        if (node->parent == node->parent->parent->left) {
            if (node == node->parent->right) {
                rbtree_rotate_left(tree, node->parent);
                node = node->left;
            }
            rbtree_rotate_right(tree, node->parent->parent);
            node->parent->color = BLACK;
            node->parent->right->color = RED;
        } else {
            if (node == node->parent->left) {
                rbtree_rotate_right(tree, node->parent);
                node = node->right;
            }
            rbtree_rotate_left(tree, node->parent->parent);
            node->parent->color = BLACK;
            node->parent->left->color = RED;
        }
    } else {
        // if uncle's color is RED
        rbtree_insert3(tree, node);
    }
}


/*
 * insert the new node.
 */
static void rbtree_insert1(struct rbtree *tree, struct rbnode *node) {
    if (tree->keycmp(node->key, node->parent->key) <= 0) {
        if (node->parent->left == NULL) {
            node->parent->left = node;
            if (node->parent->color == RED) {
                rbtree_insert2(tree, node);
            }
        } else {
            node->parent = node->parent->left;
            rbtree_insert1(tree, node);
        }
    } else {
        if (node->parent->right == NULL) {
            node->parent->right = node;
            if (node->parent->color == RED) {
                rbtree_insert2(tree, node);
            }
        } else {
            node->parent = node->parent->right;
            rbtree_insert1(tree, node);
        }
    }
}

/**
 * insert a node to a tree.
 * 
 * @param tree the root node of a tree. when inserting, the root of
 * a tree may change, so we pass a double pointer to the function.
 */
void rbtree_insert(struct rbtree *tree, void *key, void *data) {
    struct rbnode *node = malloc(sizeof(struct rbnode));
    node->key = key;
    node->data = data;
    node->color = RED;
    node->left = NULL;
    node->right = NULL;
    node->parent = NULL;
    if (tree->root == NULL) {
        node->color = BLACK;
        tree->root = node;
    } else {
        node->parent = tree->root;
        rbtree_insert1(tree, node);
    }
}
