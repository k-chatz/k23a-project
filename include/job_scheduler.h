#ifndef JOB_SCHEDULER
#define JOB_SCHEDULER

#include <stdbool.h>
#include <semaphore.h>

#define JOB_ARG(var) &(var), sizeof(var)

typedef struct job_scheduler *JobScheduler;

typedef struct job *Job;

static long long int job_id = 0;

Job js_create_job(void *(*start_routine)(void *), ...);

void js_get_arg(Job job, void * arg, int arg_index);

void js_get_args(Job job, ...);

void js_destroy_job(Job *job);

void *js_get_return_val(Job job);

long long int js_get_job_id(Job job);

void js_create(JobScheduler *js, int execution_threads);

void js_destroy(JobScheduler *js);

bool js_submit_job(JobScheduler js, Job job);

bool js_execute_all_jobs(JobScheduler js);

bool js_wait_job(Job job);

void js_wait_all_jobs(JobScheduler js);

void js_destroy(JobScheduler *js);

#endif
