#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <assert.h>
#include <unistd.h>
#include <semaphore.h>

#include "../include/queue.h"
#include "../include/colours.h"
#include "../include/job_scheduler.h"

#define QUEUE_SIZE 50

#define CRITICAL_START(MUTEX) pthread_mutex_lock(MUTEX)
#define CRITICAL_END(MUTEX) pthread_mutex_unlock(MUTEX)
#define CONDITION_WAIT(CONDITION, MUTEX) pthread_cond_wait(CONDITION, MUTEX)
#define CONDITION_SIGNAL(CONDITION) pthread_cond_signal(CONDITION)
#define CONDITION_BROADCAST(CONDITION) pthread_cond_broadcast(CONDITION)

struct job_scheduler {
    int execution_threads;
    pthread_t *tids;
    Queue waiting_queue;
    bool running;
    bool empty;
    bool exit;
    int remaining_jobs;
    pthread_cond_t condition;
    pthread_mutex_t mutex;
    sem_t sem_wait;
};

/***Private functions***/

Job js_create_job(void *(*start_routine)(void *), void *__restrict arg) {
    Job job = malloc(sizeof(struct job));
    assert(job != NULL);
    job->job_id = ++job_id;
    job->start_routine = start_routine;
    job->arg = arg;
    job->status = NULL;
    return job;
}

void *thread(JobScheduler js) {
    //sleep(1); // wait the condition var to get triggered
    while (true) {
        Job job = NULL;

        CRITICAL_START(&js->mutex);

        //while (!js->running || js->empty) {
        while (!js->running) {
            printf(B_YELLOW"[%ld] waiting for an execute signal...\n"RESET, pthread_self());

            CONDITION_WAIT(&js->condition, &js->mutex);

            if (js->exit) {
                printf(B_BLUE"[%ld] wake up from signal & exiting...\n"RESET, pthread_self());
                CRITICAL_END(&js->mutex);
                pthread_exit(NULL);
            }
            js->running = true;
        }
        //todo: use sem_wait instead of busy waiting, if(js->exit){....}

        queue_dequeue(js->waiting_queue, &job, false);
        if (job == NULL) {
            /* no more jobs */
            js->empty = true;
        }

        CRITICAL_END(&js->mutex);

        if (job != NULL) {
            /* discovering a job */
            queue_unblock_enqueue(js->waiting_queue);
            printf(B_GREEN"[%ld] starting execute job %d...\n"RESET, pthread_self(), job->job_id);
            job->status = (job->start_routine)(job);
            printf(B_GREEN"[%ld] execute job %d done! remaining_jobs:%d\n"RESET, pthread_self(), job->job_id, js->remaining_jobs-1);

            CRITICAL_START(&js->mutex);
            js->remaining_jobs--;
            CRITICAL_END(&js->mutex);
        }
    }
}

/***Public functions***/

void js_create(JobScheduler *js, int execution_threads) {
    *js = malloc(sizeof(struct job_scheduler));
    assert(*js != NULL);
    (*js)->execution_threads = execution_threads;
    queue_create(&(*js)->waiting_queue, QUEUE_SIZE, sizeof(Job));
    (*js)->running = false;
    (*js)->empty = true;
    (*js)->exit = false;
    (*js)->remaining_jobs = 0;
    (*js)->tids = malloc((*js)->execution_threads * sizeof(pthread_t));
    for (int i = 0; i < execution_threads; i++) {
        assert(!pthread_create(&(*js)->tids[i], NULL, (void *(*)(void *)) thread, *js));
    }
    /* sync */
    pthread_mutex_init(&(*js)->mutex, NULL);
    pthread_cond_init(&(*js)->condition, NULL);
    sem_init(&(*js)->sem_wait, 0, -execution_threads);
    //sleep(1); // wait the condition var to get triggered
}

bool js_submit_job(JobScheduler js, Job job) {
    //printf(B_MAGENTA"submit job %d...\n"RESET, job->job_id);
    if (js->running) {
        queue_enqueue(js->waiting_queue, &job, true);
        js->empty = false;
        js->remaining_jobs++;
        CONDITION_BROADCAST(&js->condition);
    } else {
        if (queue_is_full(js->waiting_queue, true)) {
            return false;
        }
        queue_enqueue(js->waiting_queue, &job, false);
        js->empty = false;
        js->remaining_jobs++;
    }
    return true;
}

bool js_execute_all_jobs(JobScheduler js) {
    js->running = true;
    return !CONDITION_BROADCAST(&js->condition);
}

bool js_wait_all_jobs(JobScheduler js) {
    bool ret = false;
    while (js->remaining_jobs > 0) {
        printf("remaining jobs: %d\n", js->remaining_jobs);
        sleep(1);
    }
    js->exit = true;
    CONDITION_BROADCAST(&js->condition);
    for (int i = 0; i < js->execution_threads; ++i) {
        //printf(BLUE"[%ld] trying to join...\n"RESET, js->tids[i]);
        ret += pthread_join(js->tids[i], NULL);
        //printf(BLUE"[%ld] join done...\n"RESET, js->tids[i]);
    }
    js->running = false;
    js->remaining_jobs = 0;
    return !ret;
}

void js_destroy(JobScheduler *js) {
    queue_destroy(&(*js)->waiting_queue, NULL);
    free((*js)->tids);
    free(*js);
    *js = NULL;
}
