#ifndef JOB_SCHEDULER
#define JOB_SCHEDULER

#include <stdbool.h>
#include <semaphore.h>

typedef struct job_scheduler *JobScheduler;

static int job_id = 0;

typedef struct job {
    long long int job_id;
    void *(*start_routine)(void *);
    void *__restrict arg;
    int arg_type_sz;
    void *status;
    /* sync */
    sem_t sem_complete;
} *Job;

Job js_create_job(void *(*start_routine)(void *), void *__restrict arg, int arg_type_sz);

void js_destroy_job(Job *job);

void js_create(JobScheduler *js, int execution_threads);

void js_destroy(JobScheduler *js);

bool js_submit_job(JobScheduler js, Job job);

bool js_execute_all_jobs(JobScheduler js);

bool js_wait_job(JobScheduler js, Job job);

bool js_wait_all_jobs(JobScheduler js);

#endif
