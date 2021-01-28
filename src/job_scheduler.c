#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <assert.h>
#include <semaphore.h>
#include <string.h>
#include <stdarg.h>

#include "../include/queue.h"
#include "../include/colours.h"
#include "../include/job_scheduler.h"
#include "../include/semaphore.h"

#define QUEUE_SIZE 1000000

#define LOCK_ pthread_mutex_lock(&js->mutex)
#define UNLOCK_ pthread_mutex_unlock(&js->mutex)
#define WAIT_ pthread_cond_wait(&js->condition_wake_up, &js->mutex)
#define WAIT_SUBMITTER_ pthread_cond_wait(&js->condition_wake_up_submitter, &js->mutex_submitter)
#define LOCK_SUBMITTER_ pthread_mutex_lock(&js->mutex_submitter)
#define UNLOCK_SUBMITTER_ pthread_mutex_unlock(&js->mutex_submitter)
#define BROADCAST_WAKEUP_ pthread_cond_broadcast(&js->condition_wake_up)
#define SIGNAL_WAKEUP_ pthread_cond_signal(&js->condition_wake_up)
#define RUN_ROUTINE_ job->return_val = (job->start_routine)(job)
#define EXIT_ pthread_exit(NULL)
#define NOTIFY_BARRIER_ sem_post_(js->sem_barrier)
#define NOTIFY_JOB_COMPLETE_ sem_post(&job->sem_complete)

#define FOREACH_ARG(ARG, VARGS)                                         \
void *arg;                                                              \
for (unsigned int i = 0; (arg = va_arg(vargs, void *)) != NULL; i++)    \

typedef struct argument {
    void *arg;
    size_t type_sz;
} Argument;

struct job {
    long long int job_id;

    void *(*start_routine)(void *);

    int args_count;
    Argument *args;
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
    bool working;
    bool exit;
    int ready;
    /* sync */
    pthread_cond_t condition_wake_up;
    pthread_cond_t condition_wake_up_submitter;
    pthread_mutex_t mutex;
    pthread_mutex_t mutex_submitter;
    sem_t_ *sem_barrier;
};

/***Private functions***/

void *thread(JobScheduler js) {
    int jobs_count = 0;
    while (true) {
        LOCK_;
        while ((!js->working && !js->exit) || (!queue_size(js->waiting_queue) && !js->exit)) {
            js->ready++;
            if (js->ready == js->execution_threads) {
                js->working = false;
            }
            pthread_cond_signal(&js->condition_wake_up_submitter);
            NOTIFY_BARRIER_;
            WAIT_;
            js->ready--;
        }
        if (js->exit && !queue_size(js->waiting_queue)) {
            printf(B_BLUE"Thread [%ld] exiting... (%d)\n"RESET, pthread_self(), jobs_count);
            UNLOCK_;
            EXIT_;
        }
        Job job = NULL;
        if (queue_dequeue(js->waiting_queue, &job, false)) {
            jobs_count++;
            queue_enqueue(js->running_queue, &job, false);
            queue_unblock_enqueue(js->waiting_queue);
            UNLOCK_;
            RUN_ROUTINE_;
            job->complete = true;
            NOTIFY_JOB_COMPLETE_;
            continue;
        }
        UNLOCK_;
    }
}

bool js_join_threads(JobScheduler js) {
    int ret = 0;
    sem_wait_(js->sem_barrier, false);
    LOCK_;
    js->exit = true;
    js->working = false;
    UNLOCK_;
    BROADCAST_WAKEUP_;
    for (int i = 0; i < js->execution_threads; ++i) {
        ret += pthread_join(js->tids[i], NULL);
    }
    return !ret;
}

/***Public functions***/

void js_create_job(Job *job, void *(*start_routine)(void *), ...) {
    assert(*job == NULL);
    static long long int job_id = 0;
    va_list vargs;
    *job = malloc(sizeof(struct job));
    assert(job != NULL);
    (*job)->job_id = ++job_id;
    (*job)->start_routine = start_routine;
    (*job)->args = NULL;
    (*job)->args_count = 0;
    va_start(vargs, start_routine);
    FOREACH_ARG(arg, vargs) {
        size_t type_sz = va_arg(vargs, size_t);
        (*job)->args = realloc((*job)->args, (i + 1) * sizeof(Argument));
        (*job)->args[i].arg = malloc(type_sz);
        assert((*job)->args[i].arg != NULL);
        memcpy((*job)->args[i].arg, arg, type_sz);
        (*job)->args[i].type_sz = type_sz;
        (*job)->args_count++;
    };
    va_end(vargs);
    (*job)->return_val = NULL;
    (*job)->complete = false;
    sem_init(&(*job)->sem_complete, 0, 0);
}

void js_get_arg(Job job, void *arg, int arg_index) {
    assert(job != NULL);
    memcpy(arg, job->args[arg_index].arg, job->args[arg_index].type_sz);
}

void js_get_args(Job job, ...) {
    assert(job != NULL);
    va_list vargs;
    va_start(vargs, job);
    FOREACH_ARG(arg, vargs) {
        memcpy(arg, job->args[i].arg, job->args[i].type_sz);
    };
    va_end(vargs);
}

void *js_get_return_val(JobScheduler js, Job job) {
    assert(js != NULL);
    assert(job != NULL);
    js_wait_job(js, job, false);
    return job->return_val;
}

long long int js_get_job_id(Job job) {
    assert(job != NULL);
    return job->job_id;
}

void js_destroy_job(Job *job) {
    assert(*job != NULL);
    for (int i = 0; i < (*job)->args_count; ++i) {
        free((*job)->args[i].arg);
    }
    free((*job)->args);
    if ((*job)->return_val != NULL) {
        free((*job)->return_val);
    }
    free(*job);
    *job = NULL;
}

void js_create(JobScheduler *js, int execution_threads) {
    assert(*js == NULL);
    assert(execution_threads);
    *js = malloc(sizeof(struct job_scheduler));
    assert(*js != NULL);
    (*js)->execution_threads = execution_threads;
    (*js)->waiting_queue = NULL;
    (*js)->running_queue = NULL;
    (*js)->ready = 0;
    (*js)->working = false;
    (*js)->exit = false;
    queue_create(&(*js)->waiting_queue, QUEUE_SIZE, sizeof(Job));
    queue_create(&(*js)->running_queue, QUEUE_SIZE, sizeof(Job));
    (*js)->tids = malloc((*js)->execution_threads * sizeof(pthread_t));
    /* sync */
    (*js)->sem_barrier = sem_init_(-execution_threads + 1);
    pthread_mutex_init(&(*js)->mutex, NULL);
    pthread_mutex_init(&(*js)->mutex_submitter, NULL);
    pthread_cond_init(&(*js)->condition_wake_up, NULL);
    pthread_cond_init(&(*js)->condition_wake_up_submitter, NULL);
    for (int i = 0; i < execution_threads; i++) {
        assert(!pthread_create(&(*js)->tids[i], NULL, (void *(*)(void *)) thread, *js));
    }
    sem_wait_((*js)->sem_barrier, false);
}

uint js_get_execution_threads(JobScheduler js) {
    assert(js != NULL);
    return js->execution_threads;
}

bool js_submit_job(JobScheduler js, Job job, bool force_execute) {
    assert(js != NULL);
    LOCK_;
    if (js->working || force_execute) {
        UNLOCK_;
        LOCK_SUBMITTER_;
        LOCK_;
        while (!js->ready) {
            UNLOCK_;
            WAIT_SUBMITTER_;
            LOCK_;
        }
        queue_enqueue(js->waiting_queue, &job, true);
        UNLOCK_;
        SIGNAL_WAKEUP_;
        UNLOCK_SUBMITTER_;
    } else {
        UNLOCK_;
        if (queue_is_full(js->waiting_queue, true)) {
            return false;
        }
        queue_enqueue(js->waiting_queue, &job, false);
    }
    return true;
}

Job js_create_and_submit_job(JobScheduler js, void *(*start_routine)(void *), bool force_execute, ...) {
    Job job = NULL;
    static long long int job_id = 0;
    va_list vargs;
    job = malloc(sizeof(struct job));
    assert(job != NULL);
    (job)->job_id = ++job_id;
    (job)->start_routine = start_routine;
    (job)->args = NULL;
    (job)->args_count = 0;
    va_start(vargs, force_execute);
    FOREACH_ARG(arg, vargs) {
        size_t type_sz = va_arg(vargs, size_t);
        (job)->args = realloc((job)->args, (i + 1) * sizeof(Argument));
        (job)->args[i].arg = malloc(type_sz);
        assert((job)->args[i].arg != NULL);
        memcpy((job)->args[i].arg, arg, type_sz);
        (job)->args[i].type_sz = type_sz;
        (job)->args_count++;
    };
    va_end(vargs);
    (job)->return_val = NULL;
    (job)->complete = false;
    sem_init(&(job)->sem_complete, 0, 0);

    js_submit_job(js, job, force_execute);
    return job;
}


bool js_execute_all_jobs(JobScheduler js) {
    assert(js != NULL);
    LOCK_;
    if (js->working) {
        UNLOCK_;
        return false;
    } else if (js->ready == js->execution_threads) {
        js->working = true;
        UNLOCK_;
        return !BROADCAST_WAKEUP_;
    }
    return false;
}

bool js_wait_job(JobScheduler js, Job job, bool destroy) {
    assert(js != NULL);
    LOCK_;
    if (job->complete) {
        UNLOCK_;
        if (destroy) {
            js_destroy_job(&job);
        }
        return true;
    }
    UNLOCK_;
    sem_wait(&job->sem_complete);
    if (destroy) {
        js_destroy_job(&job);
    }
    return true;
}

void js_wait_all_jobs(JobScheduler js, bool destroy_jobs) {
    assert(js != NULL);
    Job job = NULL;
    while (true) {
        LOCK_;
        if (!queue_size(js->waiting_queue) && !queue_size(js->running_queue)) {
            UNLOCK_;
            break;
        } else {
            BROADCAST_WAKEUP_;
        }
        UNLOCK_;
        job = NULL;
        LOCK_;
        queue_dequeue(js->running_queue, &job, false);
        UNLOCK_;
        if (job == NULL && !queue_size(js->waiting_queue)) {
            break;
        } else if (job == NULL) {
            continue;
        }
        js_wait_job(js, job, destroy_jobs);
    }
}

void js_destroy(JobScheduler *js) {
    assert(*js != NULL);
    js_join_threads(*js);
    queue_destroy(&(*js)->waiting_queue, NULL);
    queue_destroy(&(*js)->running_queue, NULL);
    free((*js)->tids);
    free((*js)->sem_barrier);
    free(*js);
    *js = NULL;
}
