#include <stdlib.h>

#include "queue.h"

struct queue *queue_new() {
    struct queue *q = malloc(sizeof(struct queue));
    q->size = 0;
    q->front = NULL;
    q->back = NULL;
    return q;
}

void queue_destroy(struct queue *q, void (*free_data)(void *data, void *user_data), void *user_data) {
    void *data;
    while (data = queue_pop(q)) {
        if (free_data)
            free_data(data, user_data);
    }
    free(q);
}

size_t queue_size(struct queue *q) {
    return q->size;
}

bool queue_empty(struct queue *q) {
    return q->size > 0 ? true : false;
}

void queue_push(struct queue *q, void *data) {
    struct queue_node *n = malloc(sizeof(struct queue_node));
    n->data = data;
    n->next = NULL;
    if (q->size == 0) {
        q->back = n;
        q->front = n;
    } else {
        q->back->next = n;
        q->back = n;
    }
    q->size += 1;
}

void *queue_pop(struct queue *q) {
    if (q->size > 0) {
        q->size -= 1;
        void  *data = q->front->data;
        struct queue_node *next = q->front->next;
        free(q->front);
        q->front = next;
        return data;
    }
    return NULL;
}

void *queue_front(struct queue *q) {
    return q->front ? q->front->data : NULL;
}

void *queue_back(struct queue *q) {
    return q->back ? q->back->data : NULL;
}
