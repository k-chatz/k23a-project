
#ifdef MAKEFILE
#include "../include/acutest.h"
#endif

#include "../include/hset.h"

#ifndef ACUTEST_H
#include <assert.h>

#define TEST_CHECK assert
#define TEST_ASSERT assert
#endif

void int_set_put() {
    setp S = set_new(sizeof(int));
    for (int i = 0; i < 10; i++) {
        set_put(S, &i);
    }

    bool exists[10] = {};
    keyp k;
    ulong i = 0;
    for (k = set_iterate_r(S, &i);
         k != NULL;
         k = set_iterate_r(S, &i)) {
        /* check if k is in [0, 10) */
        TEST_CHECK(*(int *) k >= 0);
        TEST_CHECK(*(int *) k < 10);
        /* check if we havent seen k before */
        TEST_CHECK(exists[*(int *) k] == false);
        exists[*(int *) k] = true;
    }

    bool all = true;
    for (i = 0; i < 10; i++) {
        all = all && exists[i];
    }
    TEST_CHECK(all);

    set_free(S);
}

void int_set_union() {
    setp A = set_new(sizeof(int));
    setp B = set_new(sizeof(int));

    for (int i = 0; i < 5; i++) {
        set_put(A, &i);
    }

    for (int i = 5; i < 10; i++) {
        set_put(B, &i);
    }

    setp S = set_union(A, B);

    bool exists[10] = {};
    keyp k;
    ulong i = 0;
    for (k = set_iterate_r(S, &i); k != NULL; k = set_iterate_r(S, &i)) {
        /* check if k is in [0, 10) */
        TEST_CHECK(*(int *) k >= 0);
        TEST_CHECK(*(int *) k < 10);
        /* check if we havent seen k before */
        TEST_CHECK(exists[*(int *) k] == false);
        exists[*(int *) k] = true;
    }

    bool all = true;
    for (i = 0; i < 10; i++) {
        all = all && exists[i];
    }
    TEST_CHECK(all);

    set_free(A);
    set_free(B);
    set_free(S);
}

#ifndef ACUTEST_H

struct test_ {
    const char *name;

    void (*func)(void);
};

#define TEST_LIST const struct test_ test_list_[]
#endif

TEST_LIST = {
        {"put",   int_set_put},
        {"union", int_set_union}
};

#ifndef ACUTEST_H

int main(int argc, char *argv[]) {
    int i;
    for (i = 0; test_list_[i].name != NULL; i++) {
        test_list_[i].func();
    }
    return 0;
}

#endif
