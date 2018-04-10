#ifndef __OSCLT_LIST_H__
#define __OSCLT_LIST_H__

struct list {
    void    *data;
    struct list *next;
};

struct list *list_new();
struct list *list_append(struct list *list, void *data);
struct list *list_remove(struct list *list, void *data);
struct list *list_extend(struct list *list1, struct list *list2);
struct list *list_next(struct list *node);
void *list_data(struct list *node);
void list_destroy(struct list *list);

#endif
