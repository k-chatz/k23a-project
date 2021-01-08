#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <assert.h>

#include "../include/queue.h"
#include "../include/job_scheduler.h"

struct thread_s {
    // state [running, waiting, complete]
    pthread_t thread_id;

} thread_t;

struct job_scheduler {
    int jobs_num;
    Queue waiting_queue;
    Queue running_queue;
};

/***Private functions***/

bool thread_init() {

}

/***Public functions***/

void js_create(JobScheduler *js, int jobs_num) {
    *js = malloc(sizeof(struct job_scheduler));
    assert(*js != NULL);
    (*js)->jobs_num = jobs_num;
    queue_create(&(*js)->waiting_queue, jobs_num, sizeof(thread_t));
    for (int i = 0; i < jobs_num; i++) {
        // queue_enqueue(js->waiting_queue, )
    }
    queue_create(&(*js)->running_queue, jobs_num, sizeof(thread_t));
}

void js_destroy(JobScheduler *js) {
    queue_destroy(&(*js)->waiting_queue);
    queue_destroy(&(*js)->running_queue);
    free(*js);
    *js = NULL;
}

bool js_submit_job(JobScheduler js, void (*job_func)(int)) {
    if (queue_is_full(js->waiting_queue)) {
        return false;
    }
    // queue_enqueue(js->waiting_queue, )
    return true;
}


