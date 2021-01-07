#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <pthread.h>

#include "../include/queue.h"
#include "../include/job-scheduler.h"

struct thread_s {
    // state [running, waiting, complete]
    pthread_t thread_id;

} thread_t;

/************************** Private *********************************/

bool thread_init(){
    
}

/************************** Public *********************************/

bool js_create(job_scheduler_t *js, int jobs_num){

    js->jobs_num = jobs_num;
    queue_create(js->waiting_queue, jobs_num, sizeof(thread_t));

    for(int i = 0; i < jobs_num; i++){ 
        
        // queue_enqueue(js->waiting_queue, )
    }

    queue_create(js->running_queue, jobs_num, sizeof(thread_t));
}

bool js_submit_job(job_scheduler_t *js, void (*job_func)(int)){

    if (queue_is_full(js->waiting_queue)){
        return false;
    }
    // queue_enqueue(js->waiting_queue, )
}


