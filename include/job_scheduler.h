#ifndef JOB_SCHEDULER
#define JOB_SCHEDULER

#include <stdbool.h>

typedef struct job_scheduler *JobScheduler;

typedef struct job Job;

void js_create(JobScheduler *js, int jobs_num);

void js_destroy(JobScheduler *js);

bool js_submit_job(JobScheduler js, const pthread_attr_t *__restrict attr, void *(*start_routine)(void *),
                   void *__restrict arg);

bool js_execute_all_jobs(JobScheduler js);

bool js_wait_all_jobs(JobScheduler js);

#endif
