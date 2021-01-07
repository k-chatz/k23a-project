#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include <assert.h>

#include "../include/queue.h"

struct queue_t {
    int front;
    int rear;
    int counter;
    int buf_sz;
    int type_sz;
    char buffer[];
};

void queue_create(Queue *q, int buf_sz, int type_sz) {
    assert(*q == NULL);
    *q = (Queue) malloc(sizeof(struct queue_t) + buf_sz * type_sz);
    assert(*q != NULL);
    (*q)->buf_sz = buf_sz;
    (*q)->type_sz = type_sz;
    (*q)->counter = 0;
    (*q)->front = 0;
    (*q)->rear = 0;
    memset((*q)->buffer, 0, (*q)->buf_sz * (*q)->type_sz);
}

void queue_destroy(Queue *q) {
    free(*q);
    *q = NULL;
}

int queue_is_empty(Queue q) {
    return (q->counter == 0);
}

int queue_size(Queue q) {
    return q->counter;
}

int queue_is_full(Queue q) {
    return (q->counter == q->buf_sz);
}

bool queue_enqueue(Queue q, void *item) {
    if (queue_is_full(q))
        return false;
    else {
        q->counter++;
        memcpy(q->buffer + (q->rear * q->type_sz), item, q->type_sz);
        q->rear = (q->rear + 1) % q->buf_sz;
    }
    return true;
}

bool queue_dequeue(Queue q, void *item) {
    if (queue_is_empty(q))
        return false;
    else {
        q->counter--;
        memcpy(item, q->buffer + (q->front * q->type_sz), q->type_sz);
        memset(q->buffer + (q->front * q->type_sz), 0, q->type_sz);
        q->front = (q->front + 1) % q->buf_sz;
    }
    return true;
}

void inspectQbyOrder(Queue q) {
    int current = q->front;
    if (!queue_is_empty(q)) {
        do {
            //CustWriteValue(stdout, &(q->array[current]));
            printf("-");
            current = (current + 1) % q->buf_sz;
        } while (current != q->rear);
        printf("\n");
    }
}

void inspectQbyArray(Queue q) {
    int i;
    for (i = 0; i < q->buf_sz; i++) {
        //CustWriteValue(stdout, &(q->array[i]));
        printf("-");
    }
    printf("\n");
}
