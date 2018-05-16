#include <stdio.h>
#include <stdlib.h>
#include "../src/list.h"

void print_int_list(struct list *list) {
    struct list *p = list;
    while (p) {
        // data
    	printf("%d ", *(int *)list_data(p));
        // next
        p = list_next(p);
    }
    printf("\n");
}

int main() {
    // new
    struct list *list = list_new();
    struct list *list1 = list_new();
    // append
    int *a1 = malloc(sizeof(int));
    int *a2 = malloc(sizeof(int));
    int *a3 = malloc(sizeof(int));
    *a1 = 13; *a2 = 6; *a3 = 9;
    list = list_append(list, a1);
    list = list_append(list, a2);
    printf("list length: %d\n", list_length(list));
    print_int_list(list);
    // extend
    list1 = list_append(list1, a3);
    list = list_extend(list, list1);
    print_int_list(list);
    // remove
    free(a1);
    list = list_remove(list, a1);
    print_int_list(list);
    // destroy
    free(a2); free(a3);
    list_destroy(list);
}
