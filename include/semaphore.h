#ifndef SEMAPHORE_H
#define SEMAPHORE_H

typedef struct semaphore sem_t_;

sem_t_ *sem_init_(int value);

void sem_decrease_(sem_t_ *s);

void sem_increase_(sem_t_ *s);

void sem_wait_(sem_t_ *s);

void sem_post_(sem_t_ *s);

long long int sem_get_value_(sem_t_ *s);

#endif
