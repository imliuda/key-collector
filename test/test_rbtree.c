#include "../src/utils/rbtree.h"

int intcmp(void *key1, void *key2) {
    return *((int *)key1) - *((int *)key2);
}

void destroy(void *key1, void *key2) {
    free(key1);
    free(key2);
}

void key(struct rbnode *node, char *key) {
    if (node == NULL)
        strcpy(key, "null");
    else
        snprintf(key, 32, "%d", *(int *)node->key);
}

void color(struct rbnode *node, char *color) {
    if (node == NULL)
        strcpy(color, "null");
    else
        snprintf(color, 32, node->color == RED ? "red" : "black");
}

void print_rbnode(struct rbnode *node) {
    char vkey[32], vparent[32], vleft[32], vright[32], vcolor[32];
    if (node != NULL) {
        key(node, vkey);
        key(node->parent, vparent);
        key(node->left, vleft);
        key(node->right, vright);
        color(node, vcolor);
        printf("current: %s, left: %s, right: %s, parent: %s, color: %s\n", vkey, vleft, vright, vparent, vcolor);

        if (node->left) {
            print_rbnode(node->left);
        }
        if (node->right) {
            print_rbnode(node->right);
        }
    }
}

void print_rbtree(struct rbtree *tree) {
    if (tree->root != NULL) {
        print_rbnode(tree->root);
    }
}

int main() {
    struct rbtree *tree;
    int a0=0,a1=1,a2=2,a3=3,a4=4,a5=5,a6=6,a7=7,a8=8,a9=9;
    int value = 100;

    printf("insert: 5, 0, 1, 6, 2, 3, 8, 4, 7, 9\n");
    tree = rbtree_new(intcmp, destroy);
    rbtree_insert(tree, &a5, &value);
    rbtree_insert(tree, &a0, &value);
    rbtree_insert(tree, &a1, &value);
    rbtree_insert(tree, &a6, &value);
    rbtree_insert(tree, &a2, &value);
    rbtree_insert(tree, &a3, &value);
    rbtree_insert(tree, &a8, &value);
    rbtree_insert(tree, &a4, &value);
    rbtree_insert(tree, &a7, &value);
    rbtree_insert(tree, &a9, &value);
    print_rbtree(tree);

    printf("\ninsert: 0, 1, 2, 3, 4, 5, 6, 7, 8, 9\n");
    tree = rbtree_new(intcmp, destroy);
    for (int i = 0; i < 10; i++) {
        int *k = malloc(sizeof(int));
        int *v = malloc(sizeof(int));
        *k = *v = i;
        rbtree_insert(tree, k, v);
    }
    print_rbtree(tree);

    printf("\ninsert: 9, 8, 7, 6, 5, 4, 3, 2, 1, 0\n");
    tree = rbtree_new(intcmp, destroy);
    for (int i = 9; i >= 0; i--) {
        int *k = malloc(sizeof(int));
        int *v = malloc(sizeof(int));
        *k = *v = i;
        rbtree_insert(tree, k, v);
    }
    print_rbtree(tree);

}
