#ifndef QUEUE_H
#define QUEUE_H

#include <stdbool.h>

typedef struct queue_t *Queue;

void queue_create(Queue *q, int buf_sz, int type_sz);

void queue_destroy(Queue *q, void (*free_t)(void *));

bool queue_is_empty(Queue q, bool sync);

int queue_size(Queue q);

bool queue_is_full(Queue q, bool sync);

bool queue_enqueue(Queue q, void *item, bool sync);

bool queue_dequeue(Queue q, void *item, bool sync);

void queue_unblock_enqueue(Queue q);

void queue_unblock_dequeue(Queue q);

void queue_inspect_by_order(Queue q, void *(*print_v)(void *));

void queue_inspect_by_array(Queue q, void *(*print_v)(void *));

#endif
