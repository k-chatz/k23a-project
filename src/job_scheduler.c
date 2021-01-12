#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <assert.h>
#include <unistd.h>

#include "../include/queue.h"
#include "../include/colours.h"
#include "../include/job_scheduler.h"

#define QUEUE_SIZE 1000

struct job_scheduler {
    int execution_threads;
    pthread_t *tids;
    Queue waiting_queue;
    Queue running_queue;
    bool execution_started;
    pthread_cond_t condition;
    pthread_cond_t condition_execute;
    pthread_mutex_t mutex;
};

Job job_new() {
    Job job = malloc(sizeof(struct job));
    assert(job != NULL);
    job->job_id = ++job_id;
    job->start_routine = NULL;
    job->arg = NULL;
    job->status = NULL;
    return job;
}

void job_destroy(Job job) {
    if ((job)->status)
        free((job)->status);
}

_Noreturn void *thread(JobScheduler js) {
    Job job = NULL;
    sleep(1);
    printf(YELLOW"[%ld] waiting for an execute signal...\n"RESET, pthread_self());
    while (1) {
        pthread_mutex_lock(&js->mutex);
        while (!js->execution_started) {
            pthread_cond_wait(&js->condition_execute, &js->mutex);
        }
        queue_dequeue(js->waiting_queue, &job);
        pthread_mutex_unlock(&js->mutex);
        printf(RED"[%ld] starting execute job %d...\n"RESET, pthread_self(), job->job_id);
        job->status = (job->start_routine)(job);
    }
    //pthread_exit(NULL);
}

void js_create(JobScheduler *js, int execution_threads) {
    *js = malloc(sizeof(struct job_scheduler));
    assert(*js != NULL);
    (*js)->execution_threads = execution_threads;
    queue_create(&(*js)->waiting_queue, QUEUE_SIZE, sizeof(Job));
    queue_create(&(*js)->running_queue, QUEUE_SIZE, sizeof(Job));
    (*js)->tids = malloc((*js)->execution_threads * sizeof(pthread_t));
    for (int i = 0; i < execution_threads; i++) {
        assert(!pthread_create(&(*js)->tids[i], NULL, (void *(*)(void *)) thread, *js));
    }
    (*js)->execution_started = false;

    /* sync */
    pthread_mutex_init(&(*js)->mutex, NULL);
    pthread_cond_init(&(*js)->condition, NULL);
    pthread_cond_init(&(*js)->condition_execute, NULL);
}

bool js_submit_job(JobScheduler js, void *(*start_routine)(void *), void *__restrict arg) {
    if (queue_is_full(js->waiting_queue, true)) {
        return false;
    }
    Job job = job_new();
    job->start_routine = start_routine;
    job->arg = arg;
    job->status = NULL;
    return queue_enqueue(js->waiting_queue, &job);
}

bool js_execute_all_jobs(JobScheduler js) {
    js->execution_started = true;
    return !pthread_cond_broadcast(&js->condition_execute);
}

bool js_wait_all_jobs(JobScheduler js) {
    Job job;
    bool ret = false;
    //todo: broadcast a stop signal
    for (int i = 0; i < js->execution_threads; ++i) {
        ret += pthread_join(js->tids[i], NULL);
    }
    js->execution_started = false;
    return !ret;
}

void js_destroy(JobScheduler *js) {
    queue_destroy(&(*js)->waiting_queue, (void (*)(void *)) job_destroy);
    queue_destroy(&(*js)->running_queue, (void (*)(void *)) job_destroy);
    free((*js)->tids);
    free(*js);
    *js = NULL;
}
