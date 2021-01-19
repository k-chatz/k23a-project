#include <pthread.h>
#include <limits.h>

#include "../include/acutest.h"
#include "../include/job_scheduler.h"
#include "../include/colours.h"

#define LOCK_ pthread_mutex_lock(&mtx)
#define UNLOCK_ pthread_mutex_unlock(&mtx)

pthread_mutex_t mtx = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t count_nonzero;

double sum = 0;

JobScheduler js = NULL;

clock_t begin, end;

/*** Thread functions ***/
/*from: https://stackoverflow.com/questions/27349480/condition-variable-example-for-pthread-library*/
void *decrement(Job job) {
    LOCK_;
    while (sum == 0) {
        pthread_cond_wait(&count_nonzero, &mtx);
    }
    sum = sum - 1;
    UNLOCK_;
    return NULL;
}

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
    double *return_val = malloc(sizeof(double));
    int i = 0, x = 0, y = 0, z = 0, sumfact = 0, smith = 0;
    int computations = 0;
    int test = 0;
    JobScheduler jobScheduler = NULL;

    js_get_args(job, &computations, &test, &jobScheduler, NULL);

    long timesec = 0;
    timesec = time(NULL);
    //srand((unsigned int) timesec);
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
    *return_val = (100.0 * smith) / i;

    LOCK_;
    if (sum == 0) {
        pthread_cond_signal(&count_nonzero);
    }
    sum += *return_val;
    UNLOCK_;

    printf(CYAN"Thread [%ld] job %lld, Computations: %d, Found %4.2f%% sum:%f\n"RESET, pthread_self(),
           js_get_job_id(job),
           computations, *return_val, sum);
    return return_val;
}

void *increment(Job job) {
    LOCK_;
    if (sum == 0) {
        pthread_cond_signal(&count_nonzero);
    }
    sum++;
    UNLOCK_;
    printf(CYAN"Thread [%ld] job %lld, sum:%f\n"RESET, pthread_self(), js_get_job_id(job), sum);
    return NULL;
}

/*** test functions ***/

void create_job_scheduler(void) {
    begin = clock();
    js_create(&js, 8);
    TEST_CHECK(js != NULL);
}

void submit_jobs(void) {
    int computations = 1000000;
    int test = 364;
    for (int j = 100000; j > 0; j -= 10000) {
        Job job = js_create_job((void *(*)(void *)) smith_numbers,
                                JOB_ARG(j),
                                JOB_ARG(test),
                                JOB_ARG(js),
                                NULL
        );
        TEST_CHECK(js_submit_job(js, job));
    }
}

void execute_all_jobs(void) {
    TEST_CHECK(js_execute_all_jobs(js));
}

void wait_all_jobs(void) {
    js_wait_all_jobs(js);
    printf(UNDERLINE BOLD"sum: %f\n"RESET, sum);
    printf("time spend: %f\n", (double) (clock() - begin) / CLOCKS_PER_SEC);
    TEST_CHECK(1);
}

void destroy_job_scheduler(void) {
    js_destroy(&js);
    TEST_CHECK(js == NULL);
}

TEST_LIST = {
        {"create_job_scheduler",  create_job_scheduler},
        {"submit_jobs",           submit_jobs},
        {"execute_all_jobs",      execute_all_jobs},
        {"wait_all_jobs",         wait_all_jobs},
        {"destroy_job_scheduler", destroy_job_scheduler},
        {NULL, NULL}
};
