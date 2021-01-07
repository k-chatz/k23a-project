#ifndef QUEUE_H
#define QUEUE_H

#include <stdbool.h>

typedef struct queue_t *Queue;

void queue_create(Queue *q, int buf_sz, int type_sz);

void queue_destroy(Queue *q);

int queue_is_empty(Queue q);

int queue_size(Queue q);

int queue_is_full(Queue q);

bool queue_enqueue(Queue q, void *item);

bool queue_dequeue(Queue q, void *item);

void inspectQbyOrder(Queue q);

void inspectQbyArray(Queue q);

#endif
