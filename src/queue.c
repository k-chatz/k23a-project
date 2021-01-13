#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include <assert.h>
#include <semaphore.h>
#include <pthread.h>

#include "../include/queue.h"

struct queue_t {
    int front;
    int rear;
    int counter;
    int buf_sz;
    int type_sz;
    bool abort_enqueue_signal;
    bool abort_dequeue_signal;
    sem_t sem_mutex;
    sem_t sem_non_empty;
    sem_t sem_non_full;
    pthread_mutex_t mutex;
    pthread_cond_t cond_non_empty;
    pthread_cond_t cond_non_full;

    char buffer[];
};

void enqueue(Queue q, void *item) {
    q->counter++;
    memcpy(q->buffer + (q->rear * q->type_sz), item, q->type_sz);
    q->rear = (q->rear + 1) % q->buf_sz;
}

void dequeue(Queue q, void *item) {
    q->counter--;
    memcpy(item, q->buffer + (q->front * q->type_sz), q->type_sz);
    memset(q->buffer + (q->front * q->type_sz), 0, q->type_sz);
    q->front = (q->front + 1) % q->buf_sz;
}

void queue_create(Queue *q, int buf_sz, int type_sz) {
    assert(*q == NULL);
    *q = (Queue) malloc(sizeof(struct queue_t) + buf_sz * type_sz);
    assert(*q != NULL);
    (*q)->buf_sz = buf_sz;
    (*q)->type_sz = type_sz;
    (*q)->counter = 0;
    (*q)->front = 0;
    (*q)->rear = 0;
    (*q)->abort_enqueue_signal = false;
    (*q)->abort_dequeue_signal = false;
    memset((*q)->buffer, 0, (*q)->buf_sz * (*q)->type_sz);

//    int err = sem_init(&(*q)->sem_mutex, 0, 1);
//    if (err) exit(-1);
//    err = sem_init(&(*q)->sem_non_empty, 0, 0);
//    if (err) exit(-1);
//    err = sem_init(&(*q)->sem_non_full, 0, buf_sz);
//    if (err) exit(-1);

    pthread_mutex_init(&(*q)->mutex, NULL);
    pthread_cond_init(&(*q)->cond_non_empty, NULL);
    pthread_cond_init(&(*q)->cond_non_full, NULL);
}

void queue_destroy(Queue *q, void (*free_t)(void *)) {
    if (free_t != NULL) {
        void *item = malloc((*q)->type_sz);
        if (free_t != NULL) {
            while (queue_dequeue(*q, item)) {
                free_t(item);
            }
        }
        free(item);
    }
    free(*q);
    *q = NULL;
}

bool queue_is_empty(Queue q, const bool sync) {
    bool is_empty = false;
    //sem_wait(&q->sem_mutex);
    if (sync) {
        pthread_mutex_lock(&q->mutex);
    }
    is_empty = (q->counter <= 0);
    if (sync) {
        pthread_mutex_unlock(&q->mutex);
    }
    //sem_post(&q->sem_mutex);
    return is_empty;
}

int queue_size(Queue q) {
    return q->counter;
}

bool queue_is_full(Queue q, const bool sync) {
    bool is_full = false;
    //sem_wait(&q->sem_mutex);
    if (sync) {
        pthread_mutex_lock(&q->mutex);
    }
    is_full = (q->counter == q->buf_sz);
    if (sync) {
        pthread_mutex_unlock(&q->mutex);
    }
    //sem_post(&q->sem_mutex);
    return is_full;
}

bool queue_enqueue(Queue q, void *item) {
    int ret = 0;
    //ret += sem_wait(&q->sem_non_full);
    //ret += sem_wait(&q->sem_mutex);
    ret += pthread_mutex_lock(&q->mutex);
    while (queue_is_full(q, false)) {
        ret += pthread_cond_wait(&q->cond_non_full, &q->mutex);
        if (q->abort_enqueue_signal) {
            q->abort_enqueue_signal = false;
            return true;
        }
    }

    /** critical section **/
    enqueue(q, item);
    /*********************/

    ret += pthread_mutex_unlock(&q->mutex);
    ret += pthread_cond_signal(&q->cond_non_empty);
    //ret += sem_post(&q->sem_mutex);
    //ret += sem_post(&q->sem_non_empty);
    return !ret;
}

bool queue_dequeue(Queue q, void *item) {
    int ret = 0;
    //ret += sem_wait(&q->sem_non_empty);
    //ret += sem_wait(&q->sem_mutex);
    ret += pthread_mutex_lock(&q->mutex);
    while (queue_is_empty(q, false)) {
        ret += pthread_cond_wait(&q->cond_non_empty, &q->mutex);
        if (q->abort_dequeue_signal) {
            q->abort_dequeue_signal = false;
            return true;
        }
    }

    /** critical section **/
    dequeue(q, item);
    /*********************/

    ret += pthread_mutex_unlock(&q->mutex);
    ret += pthread_cond_signal(&q->cond_non_full);
    //ret += sem_post(&q->sem_mutex);
    //ret += sem_post(&q->sem_non_full);
    return !ret;
}

bool queue_abort_enqueue(Queue q) {
    if (queue_is_full(q, false)) {
        q->abort_enqueue_signal = true;
        pthread_cond_signal(&q->cond_non_full);
        return true;
    }
    return false;
}

bool queue_abort_dequeue(Queue q) {
    if (queue_is_empty(q, false)) {
        q->abort_dequeue_signal = true;
        pthread_cond_broadcast(&q->cond_non_empty);
        return true;
    }
    return false;
}

void inspectQbyOrder(Queue q) {
    int current = q->front;
    if (!queue_is_empty(q, false)) {
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
