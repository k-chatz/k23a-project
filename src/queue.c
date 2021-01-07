#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include <assert.h>

#include "../include/queue.h"

enum {
    HT_ENTRY_FLAGS_CLEAR = 0,
    HT_ENTRY_FLAGS_OCCUPIED = 0b1,
    HT_ENTRY_FLAGS_DELETED = 0b10
};

struct queue_t {
    int front;
    int rear;
    int counter;
    int buf_sz;
    int type_sz;
    char array[];
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
    memset((*q)->array, 0, (*q)->buf_sz * (*q)->type_sz);
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

      //  entry_t *entry = (entry_t *) &(q->array[q->rear]);
      //  entry->flags = HT_ENTRY_FLAGS_OCCUPIED;

        memcpy(  &(q->array[q->rear]), item, q->type_sz);


        printf("\n");
        printf("%d\n", q->array[0]);
        printf("%d\n", q->array[1]);
        printf("%d\n", q->array[2]);
        printf("%d\n", q->array[3]);
        printf("%d\n", q->array[4]);
        printf("%d\n", q->array[5]);
        printf("%d\n", q->array[6]);
        printf("%d\n", q->array[7]);
        printf("%d\n", q->array[8]);
        printf("%d\n", q->array[9]);


        q->rear = (q->rear + 1) % q->buf_sz;

    }
    return true;
}

bool queue_dequeue(Queue q, void *item) {
    if (queue_is_empty(q))
        return false;
    else {
        q->counter--;
//        entry_t *entry = (entry_t *) &q->array[q->front];
//        entry->flags = HT_ENTRY_FLAGS_CLEAR;
//        memcpy(item, &entry->data, q->type_sz);
//        q->front = (q->front + 1) % q->buf_sz;
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
