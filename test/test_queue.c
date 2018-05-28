#include <stdio.h>
#include <stdlib.h>

#include "../src/queue.h"

void free_data(void *data, void *user_data) {
    free(data);
}

int main() {
    int a = 1, b = 2, c = 3;
    int *d = malloc(sizeof(int));
    int *e = malloc(sizeof(int));
    *d = 5;
    *e = 4;
    struct queue *q = queue_new();

    printf("queue empty: %d\n", queue_empty(q));

    queue_push(q, &a);
    queue_push(q, &b);
    queue_push(q, &c);

    printf("queue empty: %d, queue size: %d\n", queue_empty(q), queue_size(q));
    printf("front: %d, back: %d\n", *(int *)queue_front(q), *(int *)queue_back(q));

    void *data;
    printf("push/pop:");
    while(data = queue_pop(q)) {
        printf(" %d", *(int *)data);
    }
    printf("\n");

    queue_destroy(q, NULL, NULL);

    // destroy
    q = queue_new();
    queue_push(q, d);
    queue_push(q, e);
    queue_destroy(q, free_data, NULL);
}
