#ifndef __OSCLT_LIST_H__
#define __OSCLT_LIST_H__

typedef struct oc_list oc_list;

struct oc_list {
    void    *data;
    oc_list *prev;
    oc_list *next;
};

oc_list *oc_list_new();
void oc_list_prepend(oc_list *list, oc_list *node);
void oc_list_append(oc_list *list, oc_list *node);
void oc_list_insert_before(oc_list *cur, oc_list *node);
void oc_list_insert_after(oc_list *cur, oc_list *node);
void oc_list_remove(oc_list *list, oc_list *node);
oc_list *oc_list_first(oc_list *list);
oc_list *oc_list_last(oc_list *list);
oc_list *oc_list_prev(oc_list *list, oc_list *node);
oc_list *oc_list_next(oc_list *list, oc_list *node);
size_t oc_list_length(oc_list *list);
void oc_list_sort(oc_list *list, int (*compare)(oc_list *node1, oc_list *node2));
void oc_list_foreach(oc_list *list, void (*foreach)(oc_list *node, void *data), void *data);
void oc_list_free(oc_list *list);

#endif
