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
    Job *jobs;
    int submit_counter;
};

Job job_new() {
    Job job = malloc(sizeof(struct job));
    assert(job != NULL);
    job->thread_id = 0;
    job->attr = NULL;
    job->start_routine = NULL;
    job->arg = NULL;
    job->status = NULL;
    return job;
}

void job_destroy(Job *job) {
    assert(*job != NULL);
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
    (*js)->jobs = malloc(execution_threads * sizeof(Job));
    for (int i = 0; i < execution_threads; i++) {
        (*js)->jobs[i] = job_new();
    }
    (*js)->submit_counter = 0;
}

bool js_submit_job(JobScheduler js, const pthread_attr_t *__restrict attr, void *(*start_routine)(void *),
                   void *__restrict arg) {
    if (queue_is_full(js->waiting_queue)) {
        return false;
    } else {
        js->jobs[js->submit_counter]->attr = attr;
        js->jobs[js->submit_counter]->start_routine = start_routine;
        js->jobs[js->submit_counter]->arg = arg;
        return queue_enqueue(js->waiting_queue, &js->jobs[js->submit_counter++]);
    }
}

bool js_execute_all_jobs(JobScheduler js) {
    Job job;
    while (queue_dequeue(js->waiting_queue, &job)) {
        pthread_create(&job->thread_id, job->attr, job->start_routine, job->arg);
        //printf("create job: [%ld]\n", job->thread_id);
        queue_enqueue(js->running_queue, &job);
    }
    return true;
}

bool js_wait_all_jobs(JobScheduler js, Job **jobs) {
    Job job;
    bool ret = false;
    while (queue_dequeue(js->running_queue, &job)) {
        //printf("wait job: [%ld]\n", job->thread_id);
        ret = !pthread_join(job->thread_id, &job->status);
    }
    js->submit_counter = 0;
    *jobs = js->jobs;
    return ret;
}


void js_destroy(JobScheduler *js) {
    queue_destroy(&(*js)->waiting_queue, NULL);
    queue_destroy(&(*js)->running_queue, NULL);
    for (int i = 0; i < (*js)->execution_threads; i++) {
        if ((*js)->jobs[i]) {
            job_destroy(&(*js)->jobs[i]);
        }
    }
    free(*js);
    *js = NULL;
}