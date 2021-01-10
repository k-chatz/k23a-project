#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <assert.h>

#include "../include/queue.h"
#include "../include/job_scheduler.h"

struct job_scheduler {
    int execution_threads;
    Queue waiting_queue;
    Queue running_queue;
};

Job job_new(const pthread_attr_t *__restrict attr, void *(*start_routine)(void *), void *__restrict arg) {
    Job job = malloc(sizeof(struct job));
    assert(job != NULL);
    job->thread_id = 0;
    job->attr = attr;
    job->start_routine = start_routine;
    job->arg = arg;
    job->status = NULL;
    return job;
}

void job_destroy(Job *job){
    assert(*job!=NULL);
    if ((*job)->status)
        free((*job)->status);

    free(*job);
    *job = NULL;
}

void js_create(JobScheduler *js, int execution_threads) {
    *js = malloc(sizeof(struct job_scheduler));
    assert(*js != NULL);
    (*js)->execution_threads = execution_threads;
    queue_create(&(*js)->waiting_queue, execution_threads, sizeof(Job));
    queue_create(&(*js)->running_queue, execution_threads, sizeof(Job));
}

void js_destroy(JobScheduler *js) {
    queue_destroy(&(*js)->waiting_queue, NULL);
    queue_destroy(&(*js)->running_queue, NULL);
    free(*js);
    *js = NULL;
}

bool js_submit_job(JobScheduler js, Job job) {
    if (queue_is_full(js->waiting_queue)) {
        return false;
    } else {
        return queue_enqueue(js->waiting_queue, &job);
    }
}

bool js_execute_all_jobs(JobScheduler js) {
    Job job;
    while (queue_dequeue(js->waiting_queue, &job)) {
        pthread_create(&job->thread_id, job->attr, job->start_routine, job->arg);
        printf("create job: [%ld]\n", job->thread_id);
        queue_enqueue(js->running_queue, &job);
    }
    return true;
}

bool js_wait_all_jobs(JobScheduler js) {
    Job job;
    bool ret = false;
    while (queue_dequeue(js->running_queue, &job)) {
        printf("wait job: [%ld]\n", job->thread_id);
        ret = !pthread_join(job->thread_id, &job->status);
    }
    return ret;
}

