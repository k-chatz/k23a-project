#include "include/acutest.h"
#include "include/spec_to_specs.h"

#define N (sizeof(ids) / sizeof(ids[0]))

void print_sts(STS *sts) {
    StrList *keys = sts->keys;
    while (keys) {
        SpecEntry *sp = HT_Get(sts->ht, keys->data);
        printf("%s -> (", sp->id);
        SpecList *similar = sp->similar;
        while (similar) {
            printf("%s ", similar->data->id);
            similar = llnth(similar, 1);
        }
        printf(")\n");
        keys = llnth(keys, 1);
    }
}

void add_test(void) {
    STS *sts = sts_new();
    char *ids[] = {"www.ebay.com//123", "2", "3", "4", "5"};
    char *ids_from_sts[sizeof(ids) / sizeof(char *)];
    int i;
    for (i = 0; i < N; i++) {
        sts_add(sts, ids[i]);
    }

    i = N;
    for (StrList *key = sts->keys; key; key = llnth(key, 1)) {
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
        TEST_CHECK(strcmp(ids[1 - i], ((SpecList *) llnth(s1->similar, i))->data->id) == 0);
    }
}


TEST_LIST = {
        {"insertion", add_test},
        {"merging",   merge_test},
        {NULL,        NULL}
};
