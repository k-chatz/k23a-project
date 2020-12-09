#include <fcntl.h>
#include <unistd.h>
#include "../include/hash.h"
#include "../include/json_parser.h"
#include "../include/acutest.h"

#ifndef ACUTEST_H

#include <assert.h>

#define TEST_CHECK assert
#define TEST_ASSERT assert
#endif

#define N (sizeof(ids) / sizeof(ids[0]))

void put_get_string(void) {
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

void put_get_int_pointer() {
    int *a = NULL, *b = NULL;
    a = malloc(sizeof(int));
    *a = 10;

    printf("\na:[%p][%p][%d]\n", &a, a, *a );

    hashp json_ht = htab_new(djb2_str, 128, sizeof(int *), 10);
            TEST_ASSERT(json_ht != NULL);
            TEST_CHECK(htab_put(json_ht, "key", &a));
    b = (int *) htab_get(json_ht, "key");
            TEST_ASSERT(b != NULL);
    printf("\nb:[%p][%p][%d]\n", &b, b, *b );

    // TEST_CHECK(a == b);
}

void put_get_json_entity(void) {
    char *contents = NULL, *json_path = NULL;
    int fd = 0, bytes = 0;
    JSON_ENTITY *json_a = NULL, *json_b = NULL;
    contents = malloc(1 << 20);
    memset(contents, 0, 1 << 20);
    json_path = "tests/731.json";
    fd = open(json_path, O_RDONLY);
            TEST_ASSERT(fd > 2);
    bytes = read(fd, contents, 1 << 20);
            TEST_ASSERT(bytes > 0);
    hashp json_ht = htab_new(djb2_str, 128, sizeof(JSON_ENTITY *), 10);
            TEST_ASSERT(json_ht != NULL);
    json_a = json_to_entity(contents);
            TEST_ASSERT(json_a != NULL);
            TEST_CHECK(htab_put(json_ht, json_path, json_a));
    json_b = htab_get(json_ht, json_path);
            TEST_ASSERT(json_b != NULL);
    json_b = json_b;
            //TEST_CHECK(&(*json_a) == &(*json_b));
}

#ifndef ACUTEST_H

struct test_ {
    const char *name;

    void (*func)(void);
};

#define TEST_LIST const struct test_ test_list_[]
#endif

TEST_LIST = {
        //"put_get_string",      put_get_string},
        {"put_get_int_pointer", put_get_int_pointer},
        //{"put_get_json_entity", put_get_json_entity},
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
