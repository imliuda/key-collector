#include <stdlib.h>
#include "list.h"

/** 
 * create a new list.
 *
 * @return NULL pointer
 */
oc_list *oc_list_new() {
    oc_list *list = malloc(sizeof(oc_list));
    list->next = list;
    list->prev = list;
    return list;
}

void list_prepend(oc_list *list, oc_list *node) {
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
void oc_list_append(oc_list *list, oc_list *node) {
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
void oc_list_insert_before(oc_list *cur, oc_list *node) {
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
void oc_list_insert_after(oc_list *cur, oc_list *node) {
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
void oc_list_remove(oc_list *list, oc_list *node) {
    oc_list *p = list->next;
    // if really need check?
    while (p != list) {
        if (p == node) {
            p->prev->next = p->next;
            p->next->prev = p->prev;
        }
    }
}

/**
 * return the first element of the list.
 *
 * @param list a pointer to a list
 * @return the first element or NULL
 */
oc_list *oc_list_first(oc_list *list) {
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
oc_list *oc_list_last(oc_list *list) {
    if (list->prev == list) {
        return NULL;
    }
    return list->prev;
}

/**
 * return the next node of current node.
 *
 * @see oc_list_next
 */
oc_list *oc_list_prev(oc_list *list, oc_list *node) {
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
 * oc_list *p = list;
 * while (p = oc_list_next(list, p)) {
 *     // do something
 * }
 *
 * or use iterate a list reverse:
 *
 * oc_list *p = list;
 * while (p = oc_list_prev(list, p)) {
 *     // do something
 * }
 *
 * @param list a pointer to a list
 * @param node current node
 * @return return the next node of current or NULL if current
 * node is the last.
 */
oc_list *oc_list_next(oc_list *list, oc_list *node) {
    if (node->next == list) {
        return NULL;
    }
    return node->next;
}

size_t oc_list_length(oc_list *list) {
    size_t len = 0;
    oc_list *p = list->next;
    while (p != list) {
    	len++;
        p = p->next;
    }
    return len;
}

void oc_list_sort(oc_list *list, int (*compare)(oc_list *node1, oc_list *node2)) {

}

void oc_list_foreach(oc_list *list, void (*foreach)(oc_list *node, void *data), void *data) {
    oc_list *p = list->next;
    while (p != list) {
    	foreach(p, data);
    }
}

void oc_list_free(oc_list *list) {
    oc_list *p = list->next;
    while (p != list) {
    	free(p);
    }
    free(list);
}
