#ifndef __OSCLT_LIST_H__
#define __OSCLT_LIST_H__

struct list {
    void    *data;
    struct list *prev;
    struct list *next;
};

struct list *list_new();
void list_prepend(struct list *list, struct list *node);
void list_append(struct list *list, struct list *node);
void list_insert_before(struct list *cur, struct list *node);
void list_insert_after(struct list *cur, struct list *node);
void list_remove(struct list *list, struct list *node);
struct list *list_first(struct list *list);
struct list *list_last(struct list *list);
struct list *list_prev(struct list *list, struct list *node);
struct list *list_next(struct list *list, struct list *node);
size_t list_length(struct list *list);
void list_sort(struct list *list, int (*compare)(struct list *node1, struct list *node2));
void list_foreach(struct list *list, void (*foreach)(struct list *node, void *data), void *data);
void list_destroy(struct list *list);

#endif
