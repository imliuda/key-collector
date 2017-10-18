#include <stdlib.h>
#include "list.h"

list *list_new() {
    list *list = malloc(sizeof(list));
    list->head = NULL;
    return list;
}

void list_append(list *list, list_node *node) {
    if (list->head == NULL) {
        list->head = node;
        list->head->next = list->head;
        list->head->prev = list->head;
    } else {
        node->prev = list->head->prev;
        list->head->prev->next = node;
        node->next = list->head;
        list->head->prev = node;
    }
}

size_t list_length(list *list) {
    size_t len = 0;
    list_node *p = list->head;
    if (p == NULL) {
        return 0;
    }
    do {
        len++;
        p = p->next;
    } while (p != list->head);
    return len;
}

list_node *list_node_new(void *value) {
    list_node *node = malloc(sizeof(list_node));
    node->value = NULL;
    return node;
}

void list_node_free(list_node *node) {
    free(node);
}

void *list_node_value(list_node *node) {
    return node->value;
}
