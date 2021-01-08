#ifndef JOB_SCHEDULER
#define JOB_SCHEDULER

#include <stdbool.h>

typedef struct job_scheduler *JobScheduler;

void js_create(JobScheduler *js, int jobs_num);

void js_destroy(JobScheduler *js);

bool js_submit_job(JobScheduler js, void (*job_func)(int));

#endif
