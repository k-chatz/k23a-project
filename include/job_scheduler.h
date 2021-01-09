#ifndef JOB_SCHEDULER
#define JOB_SCHEDULER

#include <stdbool.h>

typedef struct job_scheduler *JobScheduler;
typedef struct job *Job;
typedef struct thread Thread;

Job js_create_job(void *(*start_routine)(void *), void *__restrict arg);

void js_create(JobScheduler *js, int jobs_num);

void js_destroy(JobScheduler *js);

bool js_submit_job(JobScheduler js, Job job);

#endif
