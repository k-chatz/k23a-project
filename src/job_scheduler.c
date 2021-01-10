#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <assert.h>

#include "../include/queue.h"
#include "../include/job_scheduler.h"

struct job {
    pthread_t thread_id;
    const pthread_attr_t *__restrict attr;

    void *(*start_routine)(void *);

    void *__restrict arg;
};

struct job_scheduler {
    int jobs_num;
    Queue waiting_queue;
    Queue running_queue;
};

void js_create(JobScheduler *js, int jobs_num) {
    *js = malloc(sizeof(struct job_scheduler));
    assert(*js != NULL);
    (*js)->jobs_num = jobs_num;
    queue_create(&(*js)->waiting_queue, jobs_num, sizeof(struct job));
    queue_create(&(*js)->running_queue, jobs_num, sizeof(struct job));
}

void js_destroy(JobScheduler *js) {
    queue_destroy(&(*js)->waiting_queue, NULL);
    queue_destroy(&(*js)->running_queue, NULL);
    free(*js);
    *js = NULL;
}

bool js_submit_job(JobScheduler js, const pthread_attr_t *__restrict attr, void *(*start_routine)(void *),
                   void *__restrict arg) {

    if (queue_is_full(js->waiting_queue)) {
        return false;
    } else {
        Job job;
        job.thread_id = 0;
        job.attr = attr;
        job.start_routine = start_routine;
        job.arg = arg;
        return queue_enqueue(js->waiting_queue, &job);
    }
}

bool js_execute_all_jobs(JobScheduler js) {
    Job job;
    while (queue_dequeue(js->waiting_queue, &job)) {
        pthread_create(&job.thread_id, job.attr, job.start_routine, job.arg);
        printf("create job: [%ld]\n", job.thread_id);
        queue_enqueue(js->running_queue, &job);
    }
    return true;
}

bool js_wait_all_jobs(JobScheduler js) {
    Job job;
    bool ret = false;
    while (queue_dequeue(js->running_queue, &job)) {
        printf("wait job: [%ld]\n", job.thread_id);
        void *status = NULL;
        ret = !pthread_join(job.thread_id, (void **) &status);
    }
    return ret;
}
