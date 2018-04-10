#include <stdlib.h>
#include "list.h"

/** 
 * create a new list.
 *
 * @return return a list head.
 */
struct list *list_new() {
    return NULL;
}

void *list_data(struct list *node) {
    return node->data;
}

/**
 * append an element to a list.
 *
 * @param list a pointer to a list.
 * @param data the data.
 */
struct list *list_append(struct list *list, void *data) {
    struct list *node = malloc(sizeof(struct list));
    node->data = data;
    node->next = NULL;

    if (!list) {
        return node;
    } else {
        struct list *p = list;
        while (p->next != NULL)
            p = p->next;
        p->next = node;
        return list;
    }
}

/**
 * concat two list to a list.
 */
struct list *list_extend(struct list *list1, struct list *list2) {
    struct list *p = list1;
    if (!p)
        return list2;
    while(p->next)
        p = p->next;
    p->next = list2;
    return list1;
}

/** 
 * remove an element from list.
 *
 * data must be freed before calling this function.
 */
struct list *list_remove(struct list *list, void *data) {
    struct list *next, *p = list;
    if (!p)
        return NULL;
    if (p->data == data) {
        next = p->next;
        free(p);
        return next;
    }
    next = p->next;
    while (next) {
        if (next->data == data) {
            p->next == next->next;
            free(next);
            break;
        }
        next = next->next;
        p = p->next;
    }
    return list;
}

/**
 * get the next node of current node. euqal to node->next.
 */
struct list *list_next(struct list *node) {
    return node->next;
}

/**
 * destroy a list. node data must be freed before calling this function.
 */
void list_destroy(struct list *list) {
    struct list *p = list;
    while (list != NULL) {
        p = list->next;
    	free(list);
        list = p;
    }
}
