#include <stdio.h>
#include <stdlib.h>
#include "../src/list.h"

int compare(void *data1, void *data2) {
    return *(int *)data1 - *(int *)data2;
}

void foreach(void *data, void *user_data) {
    *(int *)data += *(int *)user_data;
}

void print_int_list(struct list *list) {
    struct list *p = list;
    while (p = list_next(list, p)) {
    	printf("%d ", *(int *)p->data);
    }
    printf("\n");
}

int main() {
    struct list *node;
    struct list *list = list_new();
    for (int i = 9; i >= 0; i--) {
        int *value = malloc(sizeof(int));
        *value = i;
        list_append(list, list_node(value));
    }
    // insert
    printf("insert:\n");
    print_int_list(list);
    // length
    printf("length: %d\n", list_length(list));
    // prepend, append, insert_before, insert_after
    printf("prepend, append, insert_before, insert_after:\n");
    int a1 = 13, a2 = 6, a3 = 15, a4 = 12;
    node = list->next->next; // 8
    list_insert_before(node, list_node(&a3));
    list_insert_after(node, list_node(&a4));
    list_prepend(list, list_node(&a1));
    list_append(list, list_node(&a2));
    print_int_list(list);
    // first, last, prev, next
    printf("first: %d, last: %d, prev: %d, next: %d\n",
           *(int *)list_first(list)->data, *(int *)list_last(list)->data,
           *(int *)list_prev(list, node)->data, *(int *)list_next(list, node)->data);
    // sort
    list_sort(list, compare);
    print_int_list(list);
    // foreach
    int plus = 10;
    list_foreach(list, foreach, &plus);
    print_int_list(list);
    // remove
    list_remove(list, node);
    print_int_list(list);
}
