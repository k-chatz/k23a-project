#ifndef JOB_SCHEDULER
#define JOB_SCHEDULER

#include "../include/queue.h"

typedef struct job_scheduler_s {

    int jobs_num;
    Queue waiting_queue;
    Queue running_queue;

}job_scheduler_t;

bool js_create(job_scheduler_t *js, int jobs_num);

bool js_submit_job(job_scheduler_t *js, void (*job_func)(int));

#endif