#include "../include/acutest.h"
#include "../include/hset.h"

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

TEST_LIST = {
        {"int_set_put", int_set_put},
        {"int_set_put", int_set_put},
        {NULL, NULL}
};
