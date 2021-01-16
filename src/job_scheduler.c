#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <assert.h>
#include <unistd.h>
#include <time.h>

#include "../include/queue.h"
#include "../include/colours.h"
#include "../include/job_scheduler.h"
#include "../include/semaphore.h"

#define QUEUE_SIZE 1500

#define LOCK_ pthread_mutex_lock(&js->mutex);
#define UNLOCK_ pthread_mutex_unlock(&js->mutex);
#define WAIT_WAKEUP_SIGNAL_ pthread_cond_wait(&js->condition_wake_up, &js->mutex);
#define BROADCAST_WAKEUP_ pthread_cond_broadcast(&js->condition_wake_up);
#define SIGNAL_WAKEUP_ pthread_cond_signal(&js->condition_wake_up);

struct job_scheduler {
    uint execution_threads;
    pthread_t *tids;
    Queue queue;
    bool running;
    bool exit;
    //long long int remaining_jobs;
    pthread_cond_t condition_wake_up;
    pthread_mutex_t mutex;
    sem_t_ *sem_barrier;
    sem_t_ *sem_wakeup;
    sem_t_ *sem_jobs;
};

/***Private functions***/

void *thread(JobScheduler js) {
    int jobs_count = 0;
    bool my_turn = true;
    while (true) {
        LOCK_
        while (!js->running && !js->exit) {
            //printf(B_YELLOW"[%ld] waiting for an execution signal...\n"RESET, pthread_self());
            sem_post_(js->sem_barrier);
            WAIT_WAKEUP_SIGNAL_
            my_turn = true;
            //printf(B_RED"[%ld] execute signal arrives...\n"RESET, pthread_self());
        }
        if ((js->execution_threads > 1 && !my_turn) || queue_is_empty(js->queue, false)) {
            //printf(YELLOW"[%ld] queue is empty, waiting for jobs...\n"RESET, pthread_self());
            sem_post_(js->sem_barrier);
            sem_post_(js->sem_wakeup);
            WAIT_WAKEUP_SIGNAL_
            my_turn = true;
            //printf(B_RED"[%ld] wake up...\n"RESET, pthread_self());
        }
        if (js->exit && queue_is_empty(js->queue, false)) {
            printf(B_BLUE"[%ld] exiting... (%d)\n"RESET, pthread_self(), jobs_count);
            UNLOCK_
            pthread_exit(NULL);
        }
        Job job = NULL;
        queue_dequeue(js->queue, &job, false);
        if (job != NULL) {
            my_turn = false;
            jobs_count++;
            queue_unblock_enqueue(js->queue);
            UNLOCK_
            //printf(B_CYAN"[%ld] starting execute job %lld...\n"RESET, pthread_self(), job->job_id);
            job->status = (job->start_routine)(job);
            sem_post_(js->sem_jobs);
            //printf(WARNING"[%ld] execute job %lld done!\n"RESET, pthread_self(), job->job_id);
            BROADCAST_WAKEUP_
            continue;
        }

        UNLOCK_
    }
}

/***Public functions***/

Job js_create_job(void *(*start_routine)(void *), void *__restrict arg) {
    Job job = malloc(sizeof(struct job));
    assert(job != NULL);
    job->job_id = ++job_id;
    job->start_routine = start_routine;
    job->arg = arg;
    job->status = NULL;
    return job;
}

void js_create(JobScheduler *js, int execution_threads) {
    assert(execution_threads);
    *js = malloc(sizeof(struct job_scheduler));
    assert(*js != NULL);
    (*js)->execution_threads = execution_threads;
    queue_create(&(*js)->queue, QUEUE_SIZE, sizeof(Job));
    (*js)->running = false;
    (*js)->exit = false;
    (*js)->tids = malloc((*js)->execution_threads * sizeof(pthread_t));

    /* sync */
    (*js)->sem_barrier = sem_init_(-execution_threads + 1);
    (*js)->sem_jobs = sem_init_(1);
    (*js)->sem_wakeup = sem_init_(1);

    pthread_mutex_init(&(*js)->mutex, NULL);
    pthread_cond_init(&(*js)->condition_wake_up, NULL);

    for (int i = 0; i < execution_threads; i++) {
        assert(!pthread_create(&(*js)->tids[i], NULL, (void *(*)(void *)) thread, *js));
    }
}

bool js_submit_job(JobScheduler js, Job job) {
    if (js->running) {
        LOCK_
        sem_decrease_(js->sem_jobs);
        UNLOCK_
        queue_enqueue(js->queue, &job, true);
        sem_wait_(js->sem_wakeup, true);
        BROADCAST_WAKEUP_
    } else {
        if (queue_is_full(js->queue, true)) {
            return false;
        }
        sem_decrease_(js->sem_jobs);
        queue_enqueue(js->queue, &job, false);
    }
    return true;
}

bool js_execute_all_jobs(JobScheduler js) {
    sem_wait_(js->sem_barrier, true);
    js->running = true;
    return !BROADCAST_WAKEUP_;
}

bool js_wait_all_jobs(JobScheduler js) {
    //sleep(1);
    bool ret = false;
    //BROADCAST_WAKEUP_;
    printf(RED"\nwaiting all jobs to complete...\n"RESET);
    sem_wait_(js->sem_jobs, false);
    //printf(WARNING"all jobs completed!\n"RESET);
    LOCK_;
    js->exit = true;
    UNLOCK_;
    sem_wait_(js->sem_barrier, true);
    //BROADCAST_WAKEUP_;
    //printf(WARNING"sem_barrier!\n"RESET);

    //sleep(1);
    for (int i = 0; i < js->execution_threads; ++i) {
        BROADCAST_WAKEUP_;
        ret += pthread_join(js->tids[i], NULL);
    }
    js->running = false;
    printf(RED"join threads done!\n"RESET);
    return !ret;
}

void js_destroy(JobScheduler *js) {
    queue_destroy(&(*js)->queue, NULL);
    free((*js)->tids);
    free((*js)->sem_barrier);
    free((*js)->sem_jobs);
    free(*js);
    *js = NULL;
}
