#ifndef JOB_SCHEDULER
#define JOB_SCHEDULER

#include <stdbool.h>

typedef struct job_scheduler *JobScheduler;

static int job_id = 0;

typedef struct job {
    long long int job_id;
    void *(*start_routine)(void *);
    void *__restrict arg;
    void *status;
} *Job;

Job js_create_job(void *(*start_routine)(void *), void *__restrict arg);

void job_destroy(Job job);

void js_create(JobScheduler *js, int execution_threads);

void js_destroy(JobScheduler *js);

bool js_submit_job(JobScheduler js, Job job);

bool js_execute_all_jobs(JobScheduler js);

bool js_wait_all_jobs(JobScheduler js);

#endif
