#include <pthread.h>

#include "../include/acutest.h"
#include "../include/job_scheduler.h"
#include "../include/colours.h"

pthread_mutex_t mtx = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t count_nonzero;

unsigned count = 0;

JobScheduler js = NULL;

/*** Thread functions ***/
/*from: https://stackoverflow.com/questions/27349480/condition-variable-example-for-pthread-library*/
void *decrement(Job job) {
    int *ret = malloc(sizeof(int));
    sleep(1);
    pthread_mutex_lock(&mtx);
    //printf("[%ld] start job %d (decrement) %d\n", pthread_self(), job->job_id, count);
    while (count == 0) {
        pthread_cond_wait(&count_nonzero, &mtx);
    }
    count = count - 1;
    //printf("[%ld] end job %d (decrement) %d\n", pthread_self(), job->job_id, count);
    pthread_mutex_unlock(&mtx);
    *ret = 32;
    //pthread_exit(ret);
    return NULL;
}

void *increment(Job job) {
    int *ret = malloc(sizeof(int));
    sleep(rand() % 15 + 1);
    pthread_mutex_lock(&mtx);
    //printf("[%ld] start job %d (increment) %d\n", pthread_self(), job->job_id, count);
    if (count == 0) {
        pthread_cond_signal(&count_nonzero);
    }
    count = count + 1;
    //printf("[%ld] end job %d (increment) %d\n", pthread_self(), job->job_id, count);
    pthread_mutex_unlock(&mtx);
    *ret = 16;
    // pthread_exit(ret);
    return NULL;
}


void create_job_scheduler(void) {
    js_create(&js, 10);
    TEST_CHECK(js != NULL);
}

void submit_jobs(void) {
    for (int j = 0; j < 20; ++j) {
        TEST_CHECK(js_submit_job(js, (void *(*)(void *)) increment, NULL));
    }
    //sleep(5);

    /* overflow job scheduler queue */
    for (int j = 0; j < 5; ++j) {
         TEST_CHECK(!js_submit_job(js, (void *(*)(void *)) increment, NULL));
    }
}

void execute_all_jobs(void) {
    printf("\n");
    TEST_CHECK(js_execute_all_jobs(js));
   // sleep(20);
}

void overflow_job_scheduler(void) {
    for (int j = 0; j < 50; ++j) {
        printf(RED"inserting job...\n"RESET);
        TEST_CHECK(js_submit_job(js, (void *(*)(void *)) increment, NULL));
        printf(RED"OK!\n"RESET);
    }
}

void wait_all_jobs(void) {
    printf("\n");
    TEST_CHECK(js_wait_all_jobs(js));
    //TEST_CHECK(count == 0);
}

void destroy_job_scheduler(void) {
    js_destroy(&js);
    TEST_CHECK(js == NULL);
}

TEST_LIST = {
       {"create_job_scheduler",   create_job_scheduler},
       {"submit_jobs",            submit_jobs},
       {"execute_all_jobs",       execute_all_jobs},
       {"overflow_job_scheduler", overflow_job_scheduler},
       {"wait_all_jobs",          wait_all_jobs},
       {"destroy_job_scheduler",  destroy_job_scheduler},
        {NULL, NULL}
};
