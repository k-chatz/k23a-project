#include "../include/hash.h"

#include "../include/acutest.h"

#define N (sizeof(ids) / sizeof(ids[0]))

void put_get(void) {
    hashp hash = htab_new(djb2_str, 10, 10, 5000);
    htab_put(hash, "foo", "foo");
    char *foo = htab_get(hash, "foo");
            TEST_CHECK((strcmp("foo", foo) == 0));
}

void rehash(void) {
    hashp hash = htab_new(djb2_str, 10, 10, 50);
    htab_put(hash, "foo", "foo");

    hashp bigger = htab_new(djb2_str, 100, 100, 5000);
    char *foo = htab_get(bigger, "foo");
            TEST_CHECK((strcmp(foo, "foo") == 0));
}

TEST_LIST = {
        {"put/get", put_get},
        {NULL, NULL}
};
