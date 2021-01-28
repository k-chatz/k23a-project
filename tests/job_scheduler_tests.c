#include <pthread.h>

#include "../include/acutest.h"
#include "../include/job_scheduler.h"
#include "../include/colours.h"

#define LOCK_ pthread_mutex_lock(mtx)
#define UNLOCK_ pthread_mutex_unlock(mtx)
#define WAIT_COUNT_NON_ZERO_ pthread_cond_wait(count_nonzero, mtx)
#define SIGNAL_COUNT_NON_ZERO_ pthread_cond_signal(count_nonzero)

/*** Thread functions ***/

/*from: https://github.com/kwstarikanos/ip-smith-numbers*/
int sd(int x) {
    int sum = 0, Digit = 0;
    while (x > 0) {
        Digit = x % 10;
        x /= 10;
        sum += Digit;
    }
    return sum;
}

void *smith_numbers(Job job) {
    double *return_val = NULL, *sum = NULL;
    int i = 0, x = 0, y = 0, z = 0, sumfact = 0, smith = 0, computations = 0;
    pthread_mutex_t *mtx;
    JobScheduler js = NULL;
    js_get_args(job, &js, &sum, &computations, &mtx, NULL);
    srand(233);
    do {
        y = rand();
        z = rand();
        x = ((y % 32768) + 1) * ((z % 32768) + 1) + 1;
        sumfact = 0;
        z = x;
        for (y = 2; y * y <= z;) {
            if (z % y == 0) {
                sumfact += sd(y);
                z /= y;
            } else {
                if (y > 2)
                    y += 2;
                else
                    y = 3;
            }
        }
        if (z == 1)
            z = 0;
        if (z != 0)
            sumfact += sd(z);
        if (z == x)
            sumfact = 0;
        if (sumfact == sd(x)) {
            smith++;
            //printf("%10d is Smith number\n", x);
        }
        i++;
    } while (i <= computations);
    return_val = malloc(sizeof(double));
    *return_val = (100.0 * smith) / i;
    LOCK_;
    *sum += *return_val;
    UNLOCK_;
    printf(CYAN"Thread [%ld] job %lld, Computations: %d, Found %4.2f%% sum:%f\n"RESET, pthread_self(),
           js_get_job_id(job), computations, *return_val, *sum);
    return return_val;
}

/*from: https://stackoverflow.com/questions/27349480/condition-variable-example-for-pthread-library*/
void *decrement(Job job) {
    double *sum = 0;
    pthread_mutex_t *mtx;
    pthread_cond_t *count_nonzero;
    JobScheduler js = NULL;
    js_get_args(job, &js, &sum, &mtx, &count_nonzero, NULL);
    LOCK_;
    while (*sum == 0) {
        WAIT_COUNT_NON_ZERO_;
    }
    (*sum)--;
    UNLOCK_;
    printf(CYAN"Thread [%ld] job %lld, (decrement) sum:%f\n"RESET, pthread_self(), js_get_job_id(job), *sum);
    return NULL;
}

void *increment(Job job) {
    double *sum = 0;
    pthread_mutex_t *mtx;
    pthread_cond_t *count_nonzero;
    JobScheduler js = NULL;
    js_get_args(job, &js, &sum, &mtx, &count_nonzero, NULL);
    LOCK_;
    if (*sum == 0) {
        SIGNAL_COUNT_NON_ZERO_;
    }
    (*sum)++;
    UNLOCK_;
    printf(CYAN"Thread [%ld] job %lld, (increment) sum:%f\n"RESET, pthread_self(), js_get_job_id(job), *sum);
    return NULL;
}

/*** test functions ***/

void submit_smith_jobs(void) {
    clock_t begin = clock();
    double *sum = NULL, return_val = 0;
    pthread_mutex_t *mtx = malloc(sizeof(pthread_mutex_t));
    pthread_mutex_init(mtx, NULL);
    JobScheduler js = NULL;
    sum = malloc(sizeof(double));
    js_create(&js, 4);
    TEST_CHECK(js != NULL);
    Job jobs[2][8];
    memset(jobs, 0, sizeof(Job) * 2 * 8);
    for (int i = 0; i < 2; ++i) {
        printf(B_RED"\nstart submitting jobs...\n"RESET);
        for (int j = 0; j < 8; j++) {
            int computations = 100000 / (j + 1);
            js_create_job(&jobs[i][j], (void *(*)(void *)) smith_numbers, JOB_ARG(js), JOB_ARG(sum),
                          JOB_ARG(computations), JOB_ARG(mtx), NULL);
            TEST_CHECK(jobs[i][j] != NULL);
            TEST_CHECK(js_submit_job(js, jobs[i][j], false));
        }
        printf(RED"start execute all jobs...\n"RESET);
        js_execute_all_jobs(js);
        printf(RED"start waiting jobs...\n"RESET);
        js_wait_all_jobs(js, false);
        printf(WARNING"waiting jobs done!\n"RESET);
    }

    for (int i = 0; i < 2; ++i) {
        for (int j = 0; j < 8; j++) {
            return_val = *(double *) js_get_return_val(js, jobs[i][j]);
            printf("job's %lld return_val = [%4.2f%%]\n", js_get_job_id(jobs[i][j]), return_val);
            js_destroy_job(&jobs[i][j]);
        }
    }
    printf(UNDERLINE BOLD"sum: %f\n"RESET, *sum);
    js_destroy(&js);
    TEST_CHECK(js == NULL);
    free(sum);
    free(mtx);
    printf(WARNING"time spend: %f\n"RESET, (double) (clock() - begin) / CLOCKS_PER_SEC);
}

void submit_increment_decrement_jobs(void) {
    clock_t begin = clock();
    double *sum = NULL;
    pthread_mutex_t *mtx = malloc(sizeof(pthread_mutex_t));
    pthread_cond_t *count_nonzero = malloc(sizeof(pthread_cond_t));
    pthread_mutex_init(mtx, NULL);
    pthread_cond_init(count_nonzero, NULL);
    JobScheduler js = NULL;
    sum = malloc(sizeof(double));
    *sum = 500;
    js_create(&js, 9);
    TEST_CHECK(js != NULL);
    Job jobs[200][10];
    memset(jobs, 0, sizeof(Job) * 200 * 10);
    for (int i = 0; i < 200; ++i) {
        printf(B_RED"\nstart submitting jobs...\n"RESET);
        for (int j = 0; j < 4; j++) {
            js_create_job(&jobs[i][j], (void *(*)(void *)) increment, JOB_ARG(js), JOB_ARG(sum), JOB_ARG(mtx),
                          JOB_ARG(count_nonzero), NULL);
            TEST_CHECK(js_submit_job(js, jobs[i][j], false));
        }
        for (int j = 4; j < 10; j++) {
            js_create_job(&jobs[i][j], (void *(*)(void *)) decrement, JOB_ARG(js), JOB_ARG(sum), JOB_ARG(mtx),
                          JOB_ARG(count_nonzero), NULL);
            TEST_CHECK(js_submit_job(js, jobs[i][j], false));
        }

        printf(RED"start execute all jobs...\n"RESET);
        js_execute_all_jobs(js);
        printf(RED"start waiting jobs...\n"RESET);
        js_wait_all_jobs(js, false);
        printf(WARNING"waiting jobs done!\n"RESET);
    }
    for (int i = 0; i < 200; ++i) {
        for (int j = 0; j < 10; j++) {
            js_destroy_job(&jobs[i][j]);
        }
    }
    printf(UNDERLINE BOLD"sum: %f\n"RESET, *sum);
    TEST_CHECK(*sum == 100.0);
    js_destroy(&js);
    TEST_CHECK(js == NULL);
    free(sum);
    free(mtx);
    free(count_nonzero);
    printf(WARNING"time spend: %f\n"RESET, (double) (clock() - begin) / CLOCKS_PER_SEC);
}

TEST_LIST = {
        {"submit_smith_jobs", submit_smith_jobs},
        {"submit_increment_decrement_jobs", submit_increment_decrement_jobs},
        {NULL, NULL}
};
