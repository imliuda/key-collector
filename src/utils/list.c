#include <stdlib.h>
#include "list.h"

/** 
 * create a new list.
 *
 * @return NULL pointer
 */
struct list *list_new() {
    struct list *list = malloc(sizeof(struct list));
    list->next = list;
    list->prev = list;
    return list;
}

/*
 * create a new list node.
 */
struct list *list_node(void *data) {
    struct list *node = malloc(sizeof(struct list));
    node->data = data;
    return node;
}

void list_prepend(struct list *list, struct list *node) {
    list->next->prev = node;
    node->prev = list;
    node->next = list->next;
    list->next = node;
}

/**
 * insert a new element to a list.
 *
 * @param list a pointer to a list.
 * @param node the new element.
 */
void list_append(struct list *list, struct list *node) {
    list->prev->next = node;
    node->prev = list->prev;
    node->next = list;
    list->prev = node;
}

/**
 * insert a element before a element.
 *
 * @param cur after this element.
 * @param node the new element.
 */
void list_insert_before(struct list *cur, struct list *node) {
    cur->prev->next = node;
    node->prev = cur->prev;
    node->next = cur;
    cur->prev = node;
}

/**
 * insert a element after a element.
 *
 * @param cur after this element.
 * @param node the new element.
 */
void list_insert_after(struct list *cur, struct list *node) {
    cur->next->prev = node;
    node->next = cur->next;
    node->prev = cur;
    cur->next = node;
}

/** 
 * remove a element from list.
 *
 * @param list a pointer to a list
 * @param node node to be removed from the list
 */
void list_remove(struct list *list, struct list *node) {
    struct list *p = list->next;
    // if really need check?
    while (p != list) {
        if (p == node) {
            p->prev->next = p->next;
            p->next->prev = p->prev;
            break;
        }
        p = p->next;
    }
}

/**
 * return the first element of the list.
 *
 * @param list a pointer to a list
 * @return the first element or NULL
 */
struct list *list_first(struct list *list) {
    if (list->next == list) {
        return NULL;
    }
    return list->next;
}

/**
 * return the last element of the list.
 *
 * @param list a pointer to a list
 * @return the last element or NULL
 */
struct list *list_last(struct list *list) {
    if (list->prev == list) {
        return NULL;
    }
    return list->prev;
}

/**
 * return the next node of current node.
 *
 * @see list_next
 */
struct list *list_prev(struct list *list, struct list *node) {
    if (node->prev == list) {
        return NULL;
    }
    return node->prev;
}

/**
 * return the next node of current node.
 *
 * this function can be used to iterate a list:
 *
 * list *p = list;
 * while (p = list_next(list, p)) {
 *     // do something
 * }
 *
 * or use iterate a list reverse:
 *
 * list *p = list;
 * while (p = list_prev(list, p)) {
 *     // do something
 * }
 *
 * @param list a pointer to a list
 * @param node current node
 * @return return the next node of current or NULL if current
 * node is the last.
 */
struct list *list_next(struct list *list, struct list *node) {
    if (node->next == list) {
        return NULL;
    }
    return node->next;
}

size_t list_length(struct list *list) {
    size_t len = 0;
    struct list *p = list->next;
    while (p != list) {
    	len++;
        p = p->next;
    }
    return len;
}

void list_sort(struct list *list, int (*compare)(void *data1, void *data2)) {
    struct list *node1 = list->next, *node2;
    for (node1 = list->next; node1 != list; node1 = node1->next) {
        for (node2 = node1->next; node2 != list; node2 = node2->next) {
            if (compare(node1->data, node2->data) > 0) {
                void *tmp = node1->data;
                node1->data = node2->data;
                node2->data = tmp;
            }
        }
    }
}

void list_foreach(struct list *list, void (*foreach)(void *data, void *user_data), void *user_data) {
    struct list *p = list->next;
    while (p != list) {
    	foreach(p->data, user_data);
        p = p->next;
    }
}

void list_destroy(struct list *list) {
    struct list *p = list->next;
    while (p != list) {
    	free(p);
        p = p->next;
    }
    free(list);
}
