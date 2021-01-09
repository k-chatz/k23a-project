#include <pthread.h>

#include "../include/acutest.h"
#include "../include/job_scheduler.h"

pthread_mutex_t mtx = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t cvar1, cvar2, count_nonzero;

unsigned count = 0;

void *decrement_count(void *argp);

void *increment_count(void *argp);

void create_and_destroy_job_scheduler(void) {
    JobScheduler js = NULL;
    js_create(&js, 0);
    TEST_CHECK(js != NULL);
    js_destroy(&js);
    TEST_CHECK(js == NULL);
}

void submit_jobs(void) {
    JobScheduler js = NULL;
    js_create(&js, 10);
    TEST_CHECK(js != NULL);
    //TEST_CHECK(js_submit_job(js, js_create_job(increment_count, NULL)));
    //TEST_CHECK(js_submit_job(js, js_create_job(decrement_count, NULL)));
    TEST_CHECK(count == 0);
    js_destroy(&js);
    TEST_CHECK(js == NULL);
}

TEST_LIST = {
        {"create_and_destroy_job_scheduler", create_and_destroy_job_scheduler},
        {"submit_jobs",                      submit_jobs},
        {NULL, NULL}
};

/*** Thread functions ***/
/*used: https://stackoverflow.com/questions/27349480/condition-variable-example-for-pthread-library*/
void *decrement(void *argp) {
        sleep(1);
        pthread_mutex_lock(&mtx);
        printf("Thread %ld: start decrement_count %d\n", pthread_self(), count);
        while (count == 0) {
            pthread_cond_wait(&count_nonzero, &mtx);
        }
        count = count - 1;
        printf("Thread %ld: end decrement_count %d\n", pthread_self(), count);
        pthread_mutex_unlock(&mtx);
}

void *increment(void *argp) {
        sleep(1);
        pthread_mutex_lock(&mtx);
        printf("Thread %ld: start increment_count %d\n", pthread_self(), count);
        if (count == 0) {
            pthread_cond_signal(&count_nonzero);
        }
        count = count + 1;
        printf("Thread %ld: ent increment_count %d\n", pthread_self(), count);
        pthread_mutex_unlock(&mtx);
}