#include <pthread.h>
#include <stdlib.h>
#include <stdbool.h>

#include "../include/semaphore.h"

#define LOCK_ pthread_mutex_lock(&s->mutex);
#define UNLOCK_ pthread_mutex_unlock(&s->mutex);
#define WAIT_WAKEUP_SIGNAL_ pthread_cond_wait(&s->condition, &s->mutex);
#define SIGNAL_ pthread_cond_signal(&s->condition);

struct semaphore {
    long long int value, wakeups, initial_val;
    pthread_mutex_t mutex;
    pthread_cond_t condition;
};

sem_t_ *sem_init_(int value) {
    sem_t_ *s = malloc(sizeof(struct semaphore));
    s->value = s->initial_val = value;
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

void sem_wait_(sem_t_ *s, bool reset) {
    LOCK_
    s->value--;
    if (s->value < 0) {
        do {
            WAIT_WAKEUP_SIGNAL_
        } while (s->wakeups < 1);
        s->wakeups--;
    }
    if (reset) {
        s->value = s->initial_val;
        s->wakeups = 0;
    }
    UNLOCK_
}

void sem_post_(sem_t_ *s) {
    LOCK_
    s->value++;
    if (s->value >= 0) {
        s->wakeups++;
        SIGNAL_
    }
    UNLOCK_
}

long long int sem_get_value_(sem_t_ *s) {
    LOCK_
    return s->value;
    UNLOCK_
}

void sem_set_value_(sem_t_ *s, long long int value) {
    LOCK_
    s->value = value;
    UNLOCK_
}

void sem_reset_(sem_t_ *s) {
    LOCK_
    s->value = s->initial_val;
    UNLOCK_
}
