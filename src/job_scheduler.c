#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <assert.h>
#include <unistd.h>
#include <semaphore.h>

#include "../include/queue.h"
#include "../include/colours.h"
#include "../include/job_scheduler.h"

#define QUEUE_SIZE 15

struct job_scheduler {
    int execution_threads;
    pthread_t *tids;
    Queue waiting_queue;
    bool running;
    bool exit;
    int remaining_jobs;
    pthread_cond_t condition;
    pthread_mutex_t mutex;
    sem_t sem_wait;
};

Job job_new() {
    Job job = malloc(sizeof(struct job));
    assert(job != NULL);
    job->job_id = ++job_id;
    job->start_routine = NULL;
    job->arg = NULL;
    job->status = NULL;
    return job;
}

void *thread(JobScheduler js) {
    sleep(1); // wait the condition var to get triggered
    while (true) {
        Job job = NULL;
        pthread_mutex_lock(&js->mutex);
        while (!js->running) {
            //printf(B_YELLOW"[%ld] waiting for an execute signal...\n"RESET, pthread_self());
            pthread_cond_wait(&js->condition, &js->mutex);
            if (js->exit) {
                //printf(B_BLUE"[%ld] wake up from signal & exiting...\n"RESET, pthread_self());
                pthread_mutex_unlock(&js->mutex);
                pthread_exit(NULL);
            }
            js->running = true;
        }

        //todo: use sem_wait instead of busy waiting, if(js->exit){....}

        dequeue(js->waiting_queue, &job);
        pthread_mutex_unlock(&js->mutex);
        if (job != NULL) {
            queue_unblock_enqueue(js->waiting_queue);
            /* discovering a job */
            //printf(B_GREEN"[%ld] starting execute job %d...\n"RESET, pthread_self(), job->job_id);
            job->status = (job->start_routine)(job);
            //printf(B_GREEN"[%ld] execute job %d done!\n"RESET, pthread_self(), job->job_id);
            js->remaining_jobs--;
        } else {
            /* no more jobs */
            js->running = false;
        }
    }
}

void js_create(JobScheduler *js, int execution_threads) {
    *js = malloc(sizeof(struct job_scheduler));
    assert(*js != NULL);
    (*js)->execution_threads = execution_threads;
    queue_create(&(*js)->waiting_queue, QUEUE_SIZE, sizeof(Job));
    (*js)->tids = malloc((*js)->execution_threads * sizeof(pthread_t));
    for (int i = 0; i < execution_threads; i++) {
        assert(!pthread_create(&(*js)->tids[i], NULL, (void *(*)(void *)) thread, *js));
    }
    (*js)->running = false;
    (*js)->exit = false;
    (*js)->remaining_jobs = 0;
    /* sync */
    pthread_mutex_init(&(*js)->mutex, NULL);
    pthread_cond_init(&(*js)->condition, NULL);
    sem_init(&(*js)->sem_wait, 0, -execution_threads);
}

bool js_submit_job(JobScheduler js, void *(*start_routine)(void *), void *__restrict arg) {
    Job job = job_new();
    job->start_routine = start_routine;
    job->arg = arg;
    job->status = NULL;
    //printf(B_MAGENTA"submit job %d...\n"RESET, job->job_id);
    if(js->running){
        //printf(B_MAGENTA"oh no!, queue is full! :( waiting to enqueue job %d...\n"RESET, job->job_id);
        queue_enqueue(js->waiting_queue, &job);
        //printf(B_MAGENTA"yes! job %d was successfully enqueued...\n"RESET, job->job_id);

        // while(js->remaining_jobs >= js->execution_threads){
        //   sleep(1);
        // }

        pthread_cond_broadcast(&js->condition);
    }else{
        if (queue_is_full(js->waiting_queue, true)) {
            return false;
        }
        enqueue(js->waiting_queue, &job);
    }
    js->remaining_jobs++;
    return true;
}

bool js_execute_all_jobs(JobScheduler js) {
    js->running = true;
    return !pthread_cond_broadcast(&js->condition);
}

bool js_wait_all_jobs(JobScheduler js) {
    Job job;
    bool ret = false;

   // sem_wait(&js->sem_wait);

    while(js->remaining_jobs > 0){
        sleep(1);
    }

    js->exit = true;
    pthread_cond_broadcast(&js->condition);

    for (int i = 0; i < js->execution_threads; ++i) {
        printf(BLUE"[%ld] trying to join...\n"RESET, js->tids[i]);
        ret += pthread_join(js->tids[i], NULL);
        printf(BLUE"[%ld] join done...\n"RESET, js->tids[i]);
    }
    js->running = false;
    js->remaining_jobs = 0;
    return !ret;
}

void js_destroy(JobScheduler *js) {
    queue_destroy(&(*js)->waiting_queue, NULL);
    free((*js)->tids);
    free(*js);
    *js = NULL;
}
