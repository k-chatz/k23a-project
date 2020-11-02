#include "../include/acutest.h"
#include "../include/spec_to_specs.h"

//#include "../include/acutest.h"
#ifndef ACUTEST_H

#include <assert.h>

#define TEST_CHECK assert
#endif

#define N (sizeof(ids) / sizeof(ids[0]))

void add_test(void) {
    STS *sts = sts_new();
    char *ids[] = {"www.ebay.com//123", "2", "3", "4", "5"};
    char *ids_from_sts[sizeof(ids) / sizeof(char *)];
    int i;
    for (i = 0; i < N; i++) {
        sts_add(sts, ids[i]);
    }

    i = N;
    for (StrList *key = sts->keys; key; key = ll_nth(key, 1)) {
        ids_from_sts[--i] = key->data;
    }

    /* compare the two arrays */
    for (i = 0; i < N; i++) {
        TEST_CHECK(strcmp(ids[i], ids_from_sts[i]) == 0);
    }
}

void merge_test(void) {
    STS *sts = sts_new();
    char *ids[] = {"1", "2"};
    int i;
    for (i = 0; i < N; i++) {
        sts_add(sts, ids[i]);
    }

    sts_merge(sts, "1", "2");
    SpecEntry *s1, *s2;
    s1 = sts_get(sts, "1");
    s2 = sts_get(sts, "2");

    /* check if s1 and s2 point to the same list */
    TEST_CHECK(s1->similar == s2->similar);

    for (i = 0; i < 2; i++) {
        TEST_CHECK(strcmp(ids[1 - i], ((SpecList *) ll_nth(s1->similar, i))->data->id) == 0);
    }
}

#ifndef ACUTEST_H

struct test_ {
    const char *name;

    void (*func)(void);
};

#define TEST_LIST const struct test_ test_list_[]
#endif

TEST_LIST = {
        {"insertion", add_test},
        {"merging",   merge_test},
        {NULL, NULL}
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
