#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <assert.h>
#include <unistd.h>

#include "../include/queue.h"
#include "../include/colours.h"
#include "../include/job_scheduler.h"

#define QUEUE_SIZE 50

#define CRITICAL_START pthread_mutex_lock(&js->mutex)
#define CRITICAL_END pthread_mutex_unlock(&js->mutex)
#define CONDITION_WAIT pthread_cond_wait(&js->condition, &js->mutex)
#define CONDITION_EXECUTE_WAIT pthread_cond_wait(&js->condition_execute, &js->mutex)
#define CONDITION_SIGNAL pthread_cond_signal(&js->condition)
#define CONDITION_BROADCAST pthread_cond_broadcast(&js->condition)

typedef struct semaphore {
    int value, wakeups;
    pthread_mutex_t mutex;
    pthread_cond_t condition;
} *Semaphore;

struct job_scheduler {
    int execution_threads;
    pthread_t *tids;
    Queue waiting_queue;
    bool running;
    bool empty;
    bool exit;
    int remaining_jobs;
    pthread_cond_t condition;
    pthread_cond_t condition_execute;
    pthread_mutex_t mutex;
    Semaphore semaphore;
};

/***Private functions***/

Semaphore make_semaphore(int value) {
    Semaphore semaphore = malloc(sizeof(struct semaphore));
    semaphore->value = value;
    semaphore->wakeups = 0;
    pthread_mutex_init(&semaphore->mutex, NULL);
    pthread_cond_init(&semaphore->condition, NULL);
    return semaphore;
}

void semaphore_wait(Semaphore semaphore) {
    pthread_mutex_lock(&semaphore->mutex);
    semaphore->value--;
    if (semaphore->value < 0) {
        do {
            pthread_cond_wait(&semaphore->condition, &semaphore->mutex);
        } while (semaphore->wakeups < 1);
        semaphore->wakeups--;
    }
    pthread_mutex_unlock(&semaphore->mutex);
}

void semaphore_post(Semaphore semaphore) {
    pthread_mutex_lock(&semaphore->mutex);
    semaphore->value++;

    if (semaphore->value > 0) {
        semaphore->wakeups++;
        pthread_cond_signal(&semaphore->condition);
    }

    pthread_mutex_unlock(&semaphore->mutex);
}

void *thread(JobScheduler js) {
    while (true) {

        CRITICAL_START; ///////////////////////////////////////////////////////////////////////////////////////////////

        /* Start/stop */
        while (!js->running) {
            semaphore_post(js->semaphore);
            //printf(B_YELLOW"[%ld] waiting...\n"RESET, pthread_self());
            CONDITION_WAIT;
            //printf(B_RED"[%ld] execute signal arrives...\n"RESET, pthread_self());
        }

        /* waits until queue not empty*/
        if (js->running && js->empty ) {
            //printf(B_YELLOW"[%ld] empty queue waiting...\n"RESET, pthread_self());
            CONDITION_WAIT;
            //printf(B_RED"[%ld] queue not empty signal arrives...\n"RESET, pthread_self());
        }

        if (js->exit && js->remaining_jobs == 0) {
            //printf(B_BLUE"[%ld] wake up from signal & exiting...\n"RESET, pthread_self());
            CRITICAL_END;
            pthread_exit(NULL);
        }

        Job job = NULL;
        queue_dequeue(js->waiting_queue, &job, false);
        js->empty = job == NULL ? true : false;

        CRITICAL_END; /////////////////////////////////////////////////////////////////////////////////////////////////

        if (job != NULL) {
            /* discovering a job */
            queue_unblock_enqueue(js->waiting_queue);

            CRITICAL_START;
            js->remaining_jobs--;
            CRITICAL_END;

            //printf(B_GREEN"[%ld] starting execute job %d...\n"RESET, pthread_self(), job->job_id);
            job->status = (job->start_routine)(job);

            CRITICAL_START;
            printf(B_GREEN"[%ld] execute job %d done! remaining_jobs:%d\n"RESET, pthread_self(), job->job_id, js->remaining_jobs);
            CRITICAL_END;
        }

    }
}

/***Public functions***/

/*OK*/
Job js_create_job(void *(*start_routine)(void *), void *__restrict arg) {
    Job job = malloc(sizeof(struct job));
    assert(job != NULL);
    job->job_id = ++job_id;
    job->start_routine = start_routine;
    job->arg = arg;
    job->status = NULL;
    return job;
}

/*OK*/
void js_create(JobScheduler *js, int execution_threads) {
    *js = malloc(sizeof(struct job_scheduler));
    assert(*js != NULL);
    (*js)->execution_threads = execution_threads;
    queue_create(&(*js)->waiting_queue, QUEUE_SIZE, sizeof(Job));
    (*js)->running = false;
    (*js)->empty = true;
    (*js)->exit = false;
    (*js)->remaining_jobs = 0;
    (*js)->tids = malloc((*js)->execution_threads * sizeof(pthread_t));
    /* sync */
    (*js)->semaphore = make_semaphore(-execution_threads + 1);
    pthread_mutex_init(&(*js)->mutex, NULL);
    pthread_cond_init(&(*js)->condition, NULL);
    //sem_init(&(*js)->sem_execute, 0, 1);
    //sleep(1); // wait the condition var to get triggered
    for (int i = 0; i < execution_threads; i++) {
        assert(!pthread_create(&(*js)->tids[i], NULL, (void *(*)(void *)) thread, *js));
    }
}

/*OK*/
bool js_submit_job(JobScheduler js, Job job) {
    //printf(B_MAGENTA"submit job %d...\n"RESET, job->job_id);
    if (js->running) {
        queue_enqueue(js->waiting_queue, &job, true);
        js->empty = false;
        js->remaining_jobs++;
        CONDITION_BROADCAST;
    } else {
        if (queue_is_full(js->waiting_queue, true)) {
            return false;
        }
        queue_enqueue(js->waiting_queue, &job, false);
        js->empty = false;
        js->remaining_jobs++;
    }
    return true;
}

/*OK*/
bool js_execute_all_jobs(JobScheduler js) {
    semaphore_wait(js->semaphore);
    js->running = true;
    return !CONDITION_BROADCAST;
}

bool js_wait_all_jobs(JobScheduler js) {
    bool ret = false;
    while (js->remaining_jobs > 0) {
        printf("remaining jobs: %d\n", js->remaining_jobs);
        sleep(1);
    }
    js->exit = true;
    CONDITION_BROADCAST;
    for (int i = 0; i < js->execution_threads; ++i) {
        //printf("remaining jobs: %d\n", js->remaining_jobs);
        //printf(BLUE"[%ld] trying to join...\n"RESET, js->tids[i]);
        ret += pthread_join(js->tids[i], NULL);
        //printf(BLUE"[%ld] join done...\n"RESET, js->tids[i]);
    }
    js->empty = true;
    js->running = false;
    js->remaining_jobs = 0;
    return !ret;
}

/*OK*/
void js_destroy(JobScheduler *js) {
    queue_destroy(&(*js)->waiting_queue, NULL);
    free((*js)->tids);
    free(*js);
    *js = NULL;
}
