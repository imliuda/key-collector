#include <stdio.h>
#include "../src/utils/list.h"

int main() {
    int i = 3;
    list *list1 = list_new();
    list_append(list1, list_node_new(&i));
    printf("length: %d\n", list_length(list1));
}
