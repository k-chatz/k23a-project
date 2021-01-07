#include "../include/acutest.h"
#include "../include/queue.h"

void create_and_destroy_queue(void) {
    Queue queue = NULL;
    queue_create(&queue, 10, sizeof(int));
    TEST_CHECK(queue != NULL);
    queue_destroy(&queue);
    TEST_CHECK(queue == NULL);
}

void enqueue(void) {
    Queue queue = NULL;
    queue_create(&queue, 10, sizeof(int));
    TEST_CHECK(queue != NULL);
    for (int i = 0; i < 11; ++i) {
        if (i < 10) {
            TEST_CHECK(queue_enqueue(queue, &i));
        } else {
            TEST_CHECK(!queue_enqueue(queue, &i));
        }
    }
    queue_destroy(&queue);
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
        //TODO: TEST_CHECK(queue_dequeue(queue, &x));
        //TODO: TEST_CHECK(x == i);
    }
    queue_destroy(&queue);
    TEST_CHECK(queue == NULL);
}

void enqueue_dequeue(void) {
    TEST_CHECK(1);
}

TEST_LIST = {
        {"create_and_destroy_queue", create_and_destroy_queue},
        {"enqueue",                  enqueue},
        {"dequeue",                  dequeue},
        {"enqueue_dequeue",          enqueue_dequeue},
        {NULL, NULL}
};
