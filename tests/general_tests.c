//#include "../include/acutest.h"
#include "../include/json_parser.h"
#include <fcntl.h>

#ifndef ACUTEST_H

#include <assert.h>

#define TEST_CHECK assert
#endif

#define N (sizeof(ids) / sizeof(ids[0]))

void entities(void) {
    char *contents = malloc(1 << 20);
    char * json_path = "tests/731.json";
    int fd = open(json_path, O_RDONLY);
    int read_err = read(fd, contents, 1 << 20);
    if (read_err < 0) {
        perror("read");
    } else {
        contents[read_err] = '\0';
    }

    hashp json_ht = htab_new(djb2_str, 128, sizeof(JSON_ENTITY *), 10);

    JSON_ENTITY *json = json_to_entity(contents);

    json_print_value(json);
    putchar('\n');

    htab_put(json_ht, json_path, &json) ;

    TEST_CHECK(1);

    char * ptr = NULL;
    while((ptr = htab_iterate(json_ht))){
        JSON_ENTITY * json = (JSON_ENTITY *) (ptr + json_ht->key_sz);
        json_print_value(json);
        putchar('\n');
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
        {"entities", entities},
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
