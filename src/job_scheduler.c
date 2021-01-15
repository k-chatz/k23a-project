#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <assert.h>
#include <unistd.h>

#include "../include/queue.h"
#include "../include/colours.h"
#include "../include/job_scheduler.h"
#include "../include/semaphore.h"

#define QUEUE_SIZE 50

#define LOCK_ pthread_mutex_lock(&js->mutex);
#define UNLOCK_ pthread_mutex_unlock(&js->mutex);
#define WAIT_WAKEUP_SIGNAL_ pthread_cond_wait(&js->condition_wake_up, &js->mutex);
#define BROADCAST_WAKEUP_ pthread_cond_broadcast(&js->condition_wake_up);

struct job_scheduler {
    int execution_threads;
    pthread_t *tids;
    Queue queue;
    bool running;
    bool exit;
    int remaining_jobs;
    pthread_cond_t condition_wake_up;
    pthread_mutex_t mutex;
    sem_t_ *semaphore;
    sem_t_ *semaphore_wait;
};

/***Private functions***/

void *thread(JobScheduler js) {
    while (true) {
        LOCK_
        /* start/stop */
        while (!js->running) {
            sem_post_(js->semaphore);
            //printf(B_YELLOW"[%ld] waiting for an execution signal...\n"RESET, pthread_self());
            WAIT_WAKEUP_SIGNAL_
            //printf(B_RED"[%ld] execute signal arrives...\n"RESET, pthread_self());
        }

        /* waits until queue not empty */
        if (js->running && queue_is_empty(js->queue, false)) {
            //printf(YELLOW"[%ld] queue is empty, waiting for jobs...\n"RESET, pthread_self());
            WAIT_WAKEUP_SIGNAL_
            //printf(B_RED"[%ld] wake up...\n"RESET, pthread_self());
        }

        /* exiting */
        if (js->exit && js->remaining_jobs == 0) {
            //printf(B_BLUE"[%ld] exiting...\n"RESET, pthread_self());
            UNLOCK_
            pthread_exit(NULL);
        }

        /* dequeue job */
        Job job = NULL;
        queue_dequeue(js->queue, &job, false);
        if (job != NULL) {
            /* discovering a job */
            queue_unblock_enqueue(js->queue);
            //printf(BLUE"[%ld] semaphore_wait value: %d...\n"RESET, pthread_self(), js->semaphore_wait->value);
            //printf(BLUE"[%ld] remaining jobs: %d...\n"RESET, pthread_self(), js->remaining_jobs);
            UNLOCK_
            //printf(B_GREEN"[%ld] starting execute job %d...\n"RESET, pthread_self(), job->job_id);
            job->status = (job->start_routine)(job);
            //printf(GREEN"[%ld] execute job %d done!\n"RESET, pthread_self(), job->job_id);
            LOCK_
            js->remaining_jobs--;
            sem_post_(js->semaphore_wait);
            UNLOCK_
            continue;
        }
        UNLOCK_
    }
}

/***Public functions***/

/*OK*/
Job js_create_job(void *(*start_routine)(void *), void *__restrict arg) {
    Job job = malloc(sizeof(struct job));
    assert(job != NULL);
    job->job_id = ++job_id;
    job->start_routine = start_routine;
    job->arg = arg;
    job->status = NULL;
    return job;
}

/*OK*/
void js_create(JobScheduler *js, int execution_threads) {
    assert(execution_threads);
    *js = malloc(sizeof(struct job_scheduler));
    assert(*js != NULL);
    (*js)->execution_threads = execution_threads;
    queue_create(&(*js)->queue, QUEUE_SIZE, sizeof(Job));
    (*js)->running = false;
    (*js)->exit = false;
    (*js)->remaining_jobs = 0;
    (*js)->tids = malloc((*js)->execution_threads * sizeof(pthread_t));
    /* sync */
    (*js)->semaphore = sem_init_(-execution_threads + 1);
    (*js)->semaphore_wait = sem_init_(2);
    pthread_mutex_init(&(*js)->mutex, NULL);
    pthread_cond_init(&(*js)->condition_wake_up, NULL);
    for (int i = 0; i < execution_threads; i++) {
        assert(!pthread_create(&(*js)->tids[i], NULL, (void *(*)(void *)) thread, *js));
    }
}

/*OK*/
bool js_submit_job(JobScheduler js, Job job) {
    if (js->running) {
        queue_enqueue(js->queue, &job, true);
        LOCK_
        js->remaining_jobs++;
        sem_decrease_(js->semaphore_wait);
        UNLOCK_
        BROADCAST_WAKEUP_
    } else {
        if (queue_is_full(js->queue, true)) {
            return false;
        }
        queue_enqueue(js->queue, &job, false);
        LOCK_
        js->remaining_jobs++;
        sem_decrease_(js->semaphore_wait);
        UNLOCK_
    }
    return true;
}

/*OK*/
bool js_execute_all_jobs(JobScheduler js) {
    sem_wait_(js->semaphore);
    js->running = true;
    return !BROADCAST_WAKEUP_;
}

/*OK*/
bool js_wait_all_jobs(JobScheduler js) {
    bool ret = false;
    printf(RED"waiting jobs...\n"RESET);
    sem_wait_(js->semaphore_wait);
    printf(RED"waiting jobs done...\n"RESET);
    printf(BLUE"remaining jobs: %d\n"RESET, js->remaining_jobs);
    LOCK_;
    js->exit = true;
    UNLOCK_;
    BROADCAST_WAKEUP_;
    for (int i = 0; i < js->execution_threads; ++i) {
        printf("remaining jobs: %d\n", js->remaining_jobs);
        printf(BLUE"[%ld] trying to join...\n"RESET, js->tids[i]);
        ret += pthread_join(js->tids[i], NULL);
        printf(BLUE"[%ld] join done...\n"RESET, js->tids[i]);
    }
    js->running = false;
    js->remaining_jobs = 0;
    return !ret;
}

/*OK*/
void js_destroy(JobScheduler *js) {
    queue_destroy(&(*js)->queue, NULL);
    free((*js)->tids);
    free((*js)->semaphore);
    free((*js)->semaphore_wait);
    free(*js);
    *js = NULL;
}
