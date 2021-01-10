#ifndef JOB_SCHEDULER
#define JOB_SCHEDULER

#include <stdbool.h>

typedef struct job_scheduler *JobScheduler;

typedef struct job {
    pthread_t thread_id;
    const pthread_attr_t *__restrict attr;

    void *(*start_routine)(void *);

    void *__restrict arg;
    void *status;
} *Job;

Job job_new();

void job_destroy(Job *job);

void js_create(JobScheduler *js, int execution_threads);

void js_destroy(JobScheduler *js);

bool js_submit_job(JobScheduler js, const pthread_attr_t *__restrict attr, void *(*start_routine)(void *),
                   void *__restrict arg);

bool js_execute_all_jobs(JobScheduler js);

bool js_wait_all_jobs(JobScheduler js, Job **jobs);

#endif
