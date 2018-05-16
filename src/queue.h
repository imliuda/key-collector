#ifndef __OSCLT_QUEUE_H__
#define __OSCLT_QUEUE_H__

#include <stdbool.h>

struct queue_node {
    void *data;
    struct queue_node *next;
};

struct queue {
    size_t size;
    struct queue_node *front;
    struct queue_node *back;
};

struct queue *queue_new();
void queue_destroy(struct queue *q, void (*free_data)(void *data, void *user_data), void *user_data);
bool queue_empty(struct queue *q);
size_t queue_size(struct queue *q);
void queue_push(struct queue *q, void *data);
void *queue_pop(struct queue *q);
void *queue_front(struct queue *q);
void *queue_back(struct queue *q);

#endif
