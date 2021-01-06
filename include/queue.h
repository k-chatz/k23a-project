#ifndef QUEUE_H
#define QUEUE_H

typedef queue_t *Queue;

void QCreate(Queue *q, int buf_sz, int type_sz);

void QDestroy(Queue *q);

int QEmpty(Queue q);

int QSize(Queue q);

int QFull(Queue q);

void QInitialize(Queue q);

bool Qenqueue(Queue q, void *item);

bool Qdequeue(Queue q, void *item);

void inspectQbyOrder(Queue q);

void inspectQbyArray(Queue q);

#endif
