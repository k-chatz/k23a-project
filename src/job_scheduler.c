#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <assert.h>
#include <semaphore.h>
#include <string.h>

#include "../include/queue.h"
#include "../include/colours.h"
#include "../include/job_scheduler.h"
#include "../include/semaphore.h"

#define QUEUE_SIZE 10000

#define LOCK_ pthread_mutex_lock(&js->mutex)
#define UNLOCK_ pthread_mutex_unlock(&js->mutex)
#define WAIT_ pthread_cond_wait(&js->condition_wake_up, &js->mutex)
#define BROADCAST_WAKEUP_ pthread_cond_broadcast(&js->condition_wake_up)
#define SIGNAL_WAKEUP_ pthread_cond_signal(&js->condition_wake_up)
#define RUN_ROUTINE_ job->return_val = (job->start_routine)(job)
#define EXIT_ pthread_exit(NULL)
#define NOTIFY_BARRIER_ sem_post_(js->sem_barrier)
#define NOTIFY_JOB_COMPLETE_ sem_post(&job->sem_complete)

struct job {
    long long int job_id;

    void *(*start_routine)(void *);

    void *__restrict arg;
    int arg_type_sz;
    void *return_val;
    bool complete;
    /* sync */
    sem_t sem_complete;
};

struct job_scheduler {
    uint execution_threads;
    pthread_t *tids;
    Queue waiting_queue;
    Queue running_queue;
    bool running;
    bool exit;
    pthread_cond_t condition_wake_up;
    pthread_mutex_t mutex;
    sem_t_ *sem_barrier;
};

/***Private functions***/

void *thread(JobScheduler js) {
    int jobs_count = 0;
    while (true) {
        LOCK_;
        while ((!js->running && !js->exit) || (!queue_size(js->waiting_queue) && !js->exit)) {
            NOTIFY_BARRIER_;
            WAIT_;
        }
        if (js->exit && !queue_size(js->waiting_queue)) {
            //printf(B_BLUE"Thread [%ld] exiting... (%d)\n"RESET, pthread_self(), jobs_count);
            UNLOCK_;
            EXIT_;
        }
        Job job = NULL;
        queue_dequeue(js->waiting_queue, &job, false);
        if (job != NULL) {
            jobs_count++;
            queue_enqueue(js->running_queue, &job, true);
            queue_unblock_enqueue(js->waiting_queue);
            UNLOCK_;
            RUN_ROUTINE_;
            NOTIFY_JOB_COMPLETE_;
            continue;
        }
        UNLOCK_;
    }
}

/***Public functions***/

Job js_create_job(void *(*start_routine)(void *), void *__restrict arg, int arg_type_sz) {
    Job job = malloc(sizeof(struct job));
    assert(job != NULL);
    job->job_id = ++job_id;
    job->start_routine = start_routine;
    job->arg = malloc(arg_type_sz);
    job->arg_type_sz = arg_type_sz;
    memcpy(job->arg, arg, arg_type_sz);
    job->return_val = NULL;
    job->complete = false;
    sem_init(&job->sem_complete, 0, 0);
    return job;
}

void *js_get_job_arg(Job job) {
    return job->arg;
}

void *js_get_return_val(Job job) {
    return job->return_val;
}

long long int js_get_job_id(Job job) {
    return job->job_id;
}

void js_destroy_job(Job *job) {
    free((*job)->arg);
    free((*job));
    *job = NULL;
}

void js_create(JobScheduler *js, int execution_threads) {
    assert(execution_threads);
    *js = malloc(sizeof(struct job_scheduler));
    assert(*js != NULL);
    (*js)->execution_threads = execution_threads;
    queue_create(&(*js)->waiting_queue, QUEUE_SIZE, sizeof(Job));
    queue_create(&(*js)->running_queue, QUEUE_SIZE, sizeof(Job));
    (*js)->running = false;
    (*js)->exit = false;
    (*js)->tids = malloc((*js)->execution_threads * sizeof(pthread_t));
    /* sync */
    (*js)->sem_barrier = sem_init_(-execution_threads + 1);
    pthread_mutex_init(&(*js)->mutex, NULL);
    pthread_cond_init(&(*js)->condition_wake_up, NULL);
    for (int i = 0; i < execution_threads; i++) {
        assert(!pthread_create(&(*js)->tids[i], NULL, (void *(*)(void *)) thread, *js));
    }
    sem_wait_((*js)->sem_barrier, false);
}

bool js_submit_job(JobScheduler js, Job job) {
    if (js->running) {
        queue_enqueue(js->waiting_queue, &job, true);
        SIGNAL_WAKEUP_;
    } else {
        if (queue_is_full(js->waiting_queue, true)) {
            return false;
        }
        queue_enqueue(js->waiting_queue, &job, false);
    }
    return true;
}

bool js_execute_all_jobs(JobScheduler js) {
    js->running = true;
    return !BROADCAST_WAKEUP_;
}

bool js_wait_job(JobScheduler js, Job job) {
    if (job->complete) {
        return true;
    }
    sem_wait(&job->sem_complete);
    job->complete = true;
    return true;
}

bool js_wait_all_jobs(JobScheduler js) {
    int ret = 0;
    Job job = NULL;
    do {
        job = NULL;
        queue_dequeue(js->running_queue, &job, false);
        if (job == NULL) {
            break;
        }
        js_wait_job(js, job);
    } while (job != NULL);

    sem_wait_(js->sem_barrier, false);

    LOCK_;
    js->exit = true;
    js->running = false;
    UNLOCK_;

    BROADCAST_WAKEUP_;

    for (int i = 0; i < js->execution_threads; ++i) {
        ret += pthread_join(js->tids[i], NULL);
    }
    return !ret;
}

void js_destroy(JobScheduler *js) {
    queue_destroy(&(*js)->waiting_queue, NULL);
    queue_destroy(&(*js)->running_queue, NULL);
    free((*js)->tids);
    free((*js)->sem_barrier);
    free(*js);
    *js = NULL;
}
