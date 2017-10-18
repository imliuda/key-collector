#ifndef __OSCLT_LIST_H__
#define __OSCLT_LIST_H__

typedef struct list list;
typedef struct list_node list_node;

struct list_node {
    void        *value;
    list_node   *prev;
    list_node   *next;
};

struct list {
    list_node   *head;
};

list *list_new();
void list_prepend(list *head, list_node *node);
void list_append(list *head, list_node *node);
void list_insert_before(list_node *cur, list_node *node);
void list_insert_after(list_node *cur, list_node *node);
void list_delete(list *head, list *node);
list_node *list_first(list *head);
list_node *list_last(list *head);
list_node *list_prev(list_node *node);
list_node *list_next(list_node *node);
void list_extend(list *head1, list *head2);
size_t list_length(list *head);
void list_sort(list *head, int (*compare)(list_node *node1, list_node *node2));
void list_foreach(list *head, void (*foreach)(list_node *node));
void list_free(list *head);
list_node *list_node_new(void *value);
void list_node_free(list_node *node);
void *list_node_value(list_node *node);

#endif
