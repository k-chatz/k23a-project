#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include <assert.h>
#include <semaphore.h>

#include "../include/queue.h"

struct queue_t {
    int front;
    int rear;
    int counter;
    int buf_sz;
    int type_sz;
    sem_t *mutex;
    sem_t *non_empty;
    sem_t *non_full;
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
    (*q)->mutex = malloc(sizeof(sem_t));
    if (!(*q)->mutex){
        exit(-1);
    }
    int err = sem_init((*q)->mutex, 0, 1);
    if (err) exit(-1);

    (*q)->non_empty = malloc(sizeof(sem_t));
    if (!(*q)->non_empty){
        exit(-1);
    }
    err = sem_init((*q)->non_empty, 0, 0);
    if (err) exit(-1);

    (*q)->non_full = malloc(sizeof(sem_t));
    if (!(*q)->non_full){
        exit(-1);
    }
    err = sem_init((*q)->non_full, 0, buf_sz);
    if (err) exit(-1);
    memset((*q)->buffer, 0, (*q)->buf_sz * (*q)->type_sz);
}

void queue_destroy(Queue *q, void (*free_t)(void *)) {
    if (free_t != NULL) {
        void *item = malloc((*q)->type_sz);
        while (queue_dequeue(*q, item)) {
            free_t(item);
        }
        free(item);
    }
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
    int ret = 0;
    ret += sem_wait(q->non_full);
    ret += sem_wait(q->mutex);
    q->counter++;
    memcpy(q->buffer + (q->rear * q->type_sz), item, q->type_sz);
    q->rear = (q->rear + 1) % q->buf_sz;
    ret += sem_post(q->mutex);
    ret += sem_post(q->non_empty);
    return !ret;
}

bool queue_dequeue(Queue q, void *item) {
    int ret = 0;
    ret += sem_wait(q->non_empty);
    ret += sem_wait(q->mutex);
    q->counter--;
    memcpy(item, q->buffer + (q->front * q->type_sz), q->type_sz);
    memset(q->buffer + (q->front * q->type_sz), 0, q->type_sz);
    q->front = (q->front + 1) % q->buf_sz;
    ret += sem_post(q->mutex);
    ret += sem_post(q->non_full);
    return !ret;
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
