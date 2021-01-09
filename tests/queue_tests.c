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
    for (int i = 1; i < 12; ++i) {
        if (i < 11) {
            TEST_CHECK(queue_enqueue(queue, &i));
        } else {
            TEST_CHECK(!queue_enqueue(queue, &i));
        }
    }
    queue_destroy(&queue, NULL);
    TEST_CHECK(queue == NULL);
}

void dequeue(void) {
    Queue queue = NULL;
    queue_create(&queue, 10, sizeof(int));
    TEST_CHECK(queue != NULL);
    for (int i = 1; i < 11; ++i) {
        TEST_CHECK(queue_enqueue(queue, &i));
    }
    for (int i = 1; i < 12; ++i) {
        int x = -1;
        if (i < 11) {
            TEST_CHECK(queue_dequeue(queue, &x));
            TEST_CHECK(x == i);
        } else {
            TEST_CHECK(!queue_dequeue(queue, &x));
            TEST_CHECK(x == -1);
        }
    }
    queue_destroy(&queue, NULL);
    TEST_CHECK(queue == NULL);
}

TEST_LIST = {
        {"create_and_destroy_queue", create_and_destroy_queue},
        {"enqueue",                  enqueue},
        {"dequeue",                  dequeue},
        {NULL, NULL}
};
