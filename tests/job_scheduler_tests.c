#include <pthread.h>
#include <limits.h>

#include "../include/acutest.h"
#include "../include/job_scheduler.h"
#include "../include/colours.h"

pthread_mutex_t mtx = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t count_nonzero;

unsigned long long int sum = 0;

JobScheduler js = NULL;

clock_t begin, end;

/*** Thread functions ***/
/*from: https://stackoverflow.com/questions/27349480/condition-variable-example-for-pthread-library*/
void *decrement(Job job) {
    pthread_mutex_lock(&mtx);
    while (sum == 0) {
        pthread_cond_wait(&count_nonzero, &mtx);
    }
    sum = sum - 1;
    pthread_mutex_unlock(&mtx);
    return NULL;
}

void *increment(Job job) {
    //int sec = rand() % 2 + 1;
    //sleep(1);
    pthread_mutex_lock(&mtx);
    if (sum == 0) {
        pthread_cond_signal(&count_nonzero);
    }
    sum = sum + 1;
    printf(CYAN"[%ld] job %lld, sum:%lld\n"RESET, pthread_self(), job->job_id, sum);
    pthread_mutex_unlock(&mtx);
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
    int i = 0, x = 0, y = 0, z = 0, sumfact = 0, smith = 0;
    long timesec = 0;
    timesec = time(NULL);
    srand((unsigned int) timesec);
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
            pthread_mutex_lock(&mtx);
            if (sum == 0) {
                pthread_cond_signal(&count_nonzero);
            }
            sum += x;
            pthread_mutex_unlock(&mtx);
        }
        i++;
    } while (i <= *(int *) job->arg);
    //printf(CYAN"Thread [%ld] job %lld, Found %4.2f%% Smith numbers sum:%lld\n"RESET, pthread_self(), job->job_id, (100.0 * smith) / i, sum);
    return NULL;
}


void create_job_scheduler(void) {
    begin = clock();
    js_create(&js, 40);
    TEST_CHECK(js != NULL);
}

void submit_jobs(void) {
    int computations = 10000;
    for (int j = 0; j < 356; ++j) {
        Job job = js_create_job((void *(*)(void *)) smith_numbers, &computations, sizeof(computations));
        //Job job = js_create_job((void *(*)(void *)) decrement, NULL);
        TEST_CHECK(js_submit_job(js, job));
    }
}

void execute_all_jobs(void) {
    TEST_CHECK(js_execute_all_jobs(js));
}

void overflow_job_scheduler(void) {
    for (int j = 0; j < 0; ++j) {
        //printf(RED"inserting job...\n"RESET);
        Job job = js_create_job((void *(*)(void *)) increment, NULL, 0);
        TEST_CHECK(js_submit_job(js, job));
        //printf(RED"OK!\n"RESET);
    }
}

void wait_all_jobs(void) {
    TEST_CHECK(js_wait_all_jobs(js));
    printf(UNDERLINE BOLD"sum: %lld\n"RESET, sum);
    printf("time spend: %f\n", (double) (clock() - begin) / CLOCKS_PER_SEC);
}

void destroy_job_scheduler(void) {
    putchar('\n');
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
