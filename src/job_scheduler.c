#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <assert.h>

#include "../include/queue.h"
#include "../include/job_scheduler.h"

struct job {
    void *(*start_routine)(void *);
    void *__restrict arg;
};

struct thread {
    // state [running, waiting, complete]
    pthread_t thread_id;
    Job job;
};

struct job_scheduler {
    int jobs_num;
    Queue waiting_queue;
    Queue running_queue;
};

/***Private functions***/

void destroy_job(Job *job) {
    free(*job);
    *job = NULL;
}

void destroy_queue_item(void *item) {
    destroy_job(&((Thread *) item)->job);
}

/***Public functions***/

Job js_create_job(void *(*start_routine)(void *), void *__restrict arg) {
    Job job = malloc(sizeof(struct job));
    if (job != NULL) {
        job->start_routine = start_routine;
        job->arg = arg;
        return job;
    }
    return NULL;
}

void js_create(JobScheduler *js, int jobs_num) {
    *js = malloc(sizeof(struct job_scheduler));
    assert(*js != NULL);
    (*js)->jobs_num = jobs_num;
    queue_create(&(*js)->waiting_queue, jobs_num, sizeof(struct thread));
    for (int i = 0; i < jobs_num; i++) {
        Thread thread;
        thread.thread_id = 0;
        thread.job = NULL;
        queue_enqueue((*js)->waiting_queue, &thread);
    }
    queue_create(&(*js)->running_queue, jobs_num, sizeof(struct thread));
}

void js_destroy(JobScheduler *js) {
    queue_destroy(&(*js)->waiting_queue, destroy_queue_item);
    queue_destroy(&(*js)->running_queue, destroy_queue_item);
    free(*js);
    *js = NULL;
}

bool js_submit_job(JobScheduler js, Job job) {
    assert(job != NULL);
    if (queue_is_empty(js->waiting_queue)) {
        return false;
    } else {
        Thread thread;
        queue_dequeue(js->waiting_queue, &thread);
        thread.thread_id = 0;
        thread.job = job;
    }
    // queue_enqueue(js->waiting_queue, )
    return true;
}

bool js_execute_all_jobs(JobScheduler js, void *(*start_routine)(void *), void *__restrict arg) {

    return true;
}


bool js_wait_all_jobs(JobScheduler js, void *(*start_routine)(void *), void *__restrict arg) {

    return true;
}