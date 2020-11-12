#include "../include/hash.h"

/* #include "../include/acutest.h" */
#ifndef ACUTEST_H

#include <assert.h>

#define TEST_CHECK assert
#endif

#define N (sizeof(ids) / sizeof(ids[0]))

void put_get(void){
    hashp hash = htab_new(djb2_str, 10, 10, 5000);
    htab_put(hash, "foo", "foo");
    char *foo = htab_get(hash, "foo");
    TEST_CHECK((strcmp("foo", foo) == 0));
}

void rehash(void){
    hashp hash = htab_new(djb2_str, 10, 10, 50);
    htab_put(hash, "foo", "foo");

    hashp bigger = htab_new(djb2_str, 100, 100, 5000);
    char *foo = htab_get(bigger, "foo");
    TEST_CHECK((strcmp(foo, "foo") == 0));
}

#ifndef ACUTEST_H

struct test_ {
    const char *name;

    void (*func)(void);
};

#define TEST_LIST const struct test_ test_list_[]
#endif

TEST_LIST = {
        {"put/get", put_get},
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
