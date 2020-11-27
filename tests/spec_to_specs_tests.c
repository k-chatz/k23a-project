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
    ulong i;
    for (i = 0; i < N; i++) {
        sts_add(sts, ids[i]);
        SpecEntry *test = sts_get(sts, ids[i]);
        TEST_CHECK(strcmp(htab_get_keyp_from_valp(sts->ht->htab, test), ids[i]) == 0);
    }
    sts_destroy(sts);
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
    sts_merge(sts, "2", "3");
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
    TEST_CHECK(findRoot(sts, s1) == findRoot(sts, s2));
    TEST_CHECK(findRoot(sts, s2) == findRoot(sts, s3));
    TEST_CHECK(findRoot(sts, s3) == findRoot(sts, s6));
    TEST_CHECK(findRoot(sts, s2) == findRoot(sts, s4));
    TEST_CHECK(findRoot(sts, s4) == findRoot(sts, s5));

/*    for (int i = 0; i < 2; i++) {
                TEST_CHECK(strcmp(ids[1 - i], ((SpecList *) ll_nth(s1->similar, i))->data->id) == 0);
    }*/
    sts_destroy(sts);
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
