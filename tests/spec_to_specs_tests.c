#include "../include/spec_to_specs.h"

#include "../include/acutest.h"
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
    SpecEntry *s1, *s2, *s3, *s4, *s5, *s6;
    STS *sts = sts_new();
    char *ids[] = {"1", "2", "3", "4", "5", "6"};
    for (int i = 0; i < N; i++) {
        sts_add(sts, ids[i]);
    }
    sts_merge(sts, "1", "1");
    sts_merge(sts, "1", "2");
    sts_merge(sts, "1", "3");
    sts_merge(sts, "2", "5");
    sts_merge(sts, "3", "6");
    sts_merge(sts, "4", "6");
    sts_merge(sts, "3", "4");

    s1 = sts_get(sts, "1");
    s2 = sts_get(sts, "2");
    s3 = sts_get(sts, "3");
    s4 = sts_get(sts, "4");
    s5 = sts_get(sts, "5");
    s6 = sts_get(sts, "6");

    /* check if s1 and s2 point to the same list */
    TEST_CHECK(s1->similar == s2->similar);
            TEST_CHECK(s2->similar == s3->similar);
            TEST_CHECK(s3->similar == s6->similar);
            TEST_CHECK(s2->similar == s4->similar);
            TEST_CHECK(s4->similar == s5->similar);

/*    for (int i = 0; i < 2; i++) {
        TEST_CHECK(strcmp(ids[1 - i], ((SpecList *) ll_nth(s1->similar, i))->data->id) == 0);
    }*/
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
