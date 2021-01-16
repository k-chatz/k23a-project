#include <pthread.h>
#include <limits.h>

#include "../include/acutest.h"
#include "../include/job_scheduler.h"
#include "../include/colours.h"

pthread_mutex_t mtx = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t count_nonzero;

unsigned long long int count = 0;

JobScheduler js = NULL;

/*** Thread functions ***/
/*from: https://stackoverflow.com/questions/27349480/condition-variable-example-for-pthread-library*/
void *decrement(Job job) {
    pthread_mutex_lock(&mtx);
    while (count == 0) {
        pthread_cond_wait(&count_nonzero, &mtx);
    }
    count = count - 1;
    pthread_mutex_unlock(&mtx);
    return NULL;
}

void *increment(Job job) {
    //int sec = rand() % 2 + 1;
    //sleep(1);
    //pthread_mutex_lock(&mtx);
    if (count == 0) {
        pthread_cond_signal(&count_nonzero);
    }
    count = count + 1;
    //pthread_mutex_unlock(&mtx);
    printf(CYAN"[%ld] job %lld, count:%lld\n"RESET, pthread_self(), job->job_id, count);
    return NULL;
}

void create_job_scheduler(void) {
    js_create(&js, 8);
    TEST_CHECK(js != NULL);
}

void submit_jobs(void) {
    for (int j = 0; j < 356; ++j) {
        Job job = js_create_job((void *(*)(void *)) increment, NULL);
        //Job job = js_create_job((void *(*)(void *)) decrement, NULL);
        TEST_CHECK(js_submit_job(js, job));
    }
    /* overflow job scheduler queue */
//    for (int j = 0; j < 5; ++j) {
//        Job job = js_create_job((void *(*)(void *)) increment, NULL);
//        TEST_CHECK(!js_submit_job(js, job));
//    }
}

void execute_all_jobs(void) {
    //sleep(1);
    TEST_CHECK(js_execute_all_jobs(js));
}

void overflow_job_scheduler(void) {
    //sleep(1);
    for (int j = 0; j < 0; ++j) {
        //printf(RED"inserting job...\n"RESET);
        Job job = js_create_job((void *(*)(void *)) increment, NULL);
        TEST_CHECK(js_submit_job(js, job));
        //printf(RED"OK!\n"RESET);
    }
}

void wait_all_jobs(void) {
    //sleep(1);
    TEST_CHECK(js_wait_all_jobs(js));
    printf(UNDERLINE BOLD"count: %lld\n"RESET, count);
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
