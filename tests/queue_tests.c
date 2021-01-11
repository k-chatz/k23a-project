#include <pthread.h>

#include "../include/acutest.h"
#include "../include/queue.h"



void create_and_destroy_queue(void) {
    Queue queue = NULL;
    queue_create(&queue, 10, sizeof(int));
    TEST_CHECK(queue != NULL);
    queue_destroy(&queue, NULL);
    TEST_CHECK(queue == NULL);
}

void enqueue(void) {
    Queue queue = NULL;
    queue_create(&queue, 10, sizeof(int));
    TEST_CHECK(queue != NULL);
    for (int i = 0; i < 10; ++i) {
        TEST_CHECK(queue_enqueue(queue, &i));
    }
    queue_destroy(&queue, NULL);
    TEST_CHECK(queue == NULL);
}

void dequeue(void) {
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

void * thread_b(void * arg){
    Queue q = (Queue) arg;
    int i = 0;
    printf("Started thread b\n");
    queue_dequeue(q, &i);    
    printf("thread_b: dequeue done, %d\n", i);
}

void *thread_c(void * arg){
    Queue q = (Queue) arg;
    int i = 0;
    printf("Started thread c\n");
    queue_dequeue(q, &i);    
    printf("thread_c: dequeue done, %d\n", i);
}


void critical_section(void){
    Queue queue1 = NULL;
    pthread_t t_b, t_c;
    void *ret_b, *ret_c;
    queue_create(&queue1, 10, sizeof(int));
    TEST_CHECK(queue1 != NULL);
    pthread_create(t_b, NULL, thread_b, &queue1);
    pthread_create(t_c, NULL, thread_c, &queue1);

    sleep(5);
    printf("Woke up from sleep.\n");
    int i = 1;
    queue_enqueue(queue1, &i);
    printf("enqueueing if %d done!\n", i);
    pthread_join(t_b, ret_b);
    pthread_join(t_c, ret_c);
}

TEST_LIST = {
        {"create_and_destroy_queue", create_and_destroy_queue},
        {"enqueue",                  enqueue},
        {"dequeue",                  dequeue},
        {"critical_section", critical_section},
        {NULL, NULL}
};
