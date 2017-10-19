#include <stdio.h>
#include <stdlib.h>
#include "../src/utils/list.h"

int main() {
    int i = 3;
    oc_list *p;
    oc_list *list = oc_list_new();
    for (int i=0; i<10; i++) {
        int *value = malloc(sizeof(int));
        *value = i;
        oc_list *node = malloc(sizeof(oc_list));
        node->data = value;
        oc_list_append(list, node);
    }
    p = list;
    while (p = oc_list_next(list, p)) {
    	printf("%d\n", *(int *)p->data);
    }
    printf("length: %d\n", oc_list_length(list));
}
