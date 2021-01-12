#include <pthread.h>

#include "../include/acutest.h"
#include "../include/queue.h"

Queue q = NULL;

void create_and_destroy_queue(void) {
    Queue queue = NULL;
    queue_create(&queue, 10, sizeof(int));
    TEST_CHECK(queue != NULL);
    queue_destroy(&queue, NULL);
    TEST_CHECK(queue == NULL);
}

void enqueue_test(void) {
    Queue queue = NULL;
    queue_create(&queue, 10, sizeof(int));
    TEST_CHECK(queue != NULL);
    for (int i = 0; i < 10; ++i) {
        TEST_CHECK(queue_enqueue(queue, &i));
    }
    queue_destroy(&queue, NULL);
    TEST_CHECK(queue == NULL);
}

void dequeue_test(void) {
    Queue queue = NULL;
    queue_create(&queue, 10, sizeof(int));
    TEST_CHECK(queue != NULL);
    for (int i = 0; i < 10; ++i) {
        TEST_CHECK(queue_enqueue(queue, &i));
    }
    for (int i = 0; i < 10; ++i) {
        int x = -1;
        TEST_CHECK(queue_dequeue(queue, &x));
        TEST_CHECK(x == i);
    }
    queue_destroy(&queue, NULL);
    TEST_CHECK(queue == NULL);
}

void *queue_producer_f(void *arg) {
    int i = 0;
    sleep(rand() % 10 + 1);
    // printf("\nStarted queue_producer_f\n");
    queue_enqueue(q, &i);
    // printf("\nthread_c: dequeue done, %d\n", i);
    return NULL;
}

void *queue_consumer_f(void *arg) {
    int i = 0;
    sleep(rand() % 10 + 1);
    // printf("\nStarted queue_consumer_f\n");
    queue_dequeue(q, &i);
    // printf("\nthread_b: dequeue done, %d\n", i);
    return NULL;
}

void queue_blocking(void) {
    pthread_t t_a, t_b, t_c, t_d, t_e, t_f, t_g;
    pthread_t t_1, t_2;
    queue_create(&q, 5, sizeof(int));
    TEST_CHECK(q != NULL);
    pthread_create(&t_a, NULL, queue_producer_f, NULL);
    pthread_create(&t_b, NULL, queue_producer_f, NULL);
    pthread_create(&t_c, NULL, queue_producer_f, NULL);
    pthread_create(&t_d, NULL, queue_producer_f, NULL);
    pthread_create(&t_e, NULL, queue_producer_f, NULL);
    pthread_create(&t_f, NULL, queue_producer_f, NULL);
    pthread_create(&t_g, NULL, queue_producer_f, NULL);
    pthread_create(&t_1, NULL, queue_consumer_f, NULL);
    pthread_create(&t_2, NULL, queue_consumer_f, NULL);
    pthread_join(t_a, NULL);
    pthread_join(t_b, NULL);
    pthread_join(t_c, NULL);
    pthread_join(t_d, NULL);
    pthread_join(t_e, NULL);
    pthread_join(t_f, NULL);
    pthread_join(t_g, NULL);
    pthread_join(t_1, NULL);
}

TEST_LIST = {
        {"create_and_destroy_queue", create_and_destroy_queue},
        {"enqueue",                  enqueue_test},
        {"dequeue",                  dequeue_test},
        {"queue_blocking",           queue_blocking},
        {NULL, NULL}
};
