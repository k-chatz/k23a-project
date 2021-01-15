#include <pthread.h>
#include <stdlib.h>

#include "../include/semaphore.h"

#define LOCK_ pthread_mutex_lock(&s->mutex);
#define UNLOCK_ pthread_mutex_unlock(&s->mutex);
#define WAIT_ pthread_cond_wait(&s->condition, &s->mutex);
#define SIGNAL_ pthread_cond_signal(&s->condition);

struct semaphore {
    int value, wakeups;
    pthread_mutex_t mutex;
    pthread_cond_t condition;
};

sem_t_ *sem_init_(int value) {
    sem_t_ *s = malloc(sizeof(struct semaphore));
    s->value = value;
    s->wakeups = 0;
    pthread_mutex_init(&s->mutex, NULL);
    pthread_cond_init(&s->condition, NULL);
    return s;
}

void sem_decrease_(sem_t_ *s) {
    LOCK_
    s->value--;
    UNLOCK_
}

void sem_increase_(sem_t_ *s) {
    LOCK_
    s->value++;
    UNLOCK_
}

void sem_wait_(sem_t_ *s) {
    LOCK_
    s->value--;
    if (s->value < 0) {
        do {
            WAIT_
        } while (s->wakeups < 1);
        s->wakeups--;
    }
    UNLOCK_
}

void sem_post_(sem_t_ *s) {
    LOCK_
    s->value++;
    if (s->value > 0) {
        s->wakeups++;
        SIGNAL_
    }
    UNLOCK_
}

int sem_get_value_(sem_t_ *s) {
    LOCK_
    return s->value;
    UNLOCK_
}
