#include "../include/acutest.h"

#ifndef ACUTEST_H

#include <assert.h>

#define TEST_CHECK assert
#define TEST_ASSERT assert
#endif

#define N (sizeof(ids) / sizeof(ids[0]))

void test(void) {

}

#ifndef ACUTEST_H

struct test_ {
    const char *name;

    void (*func)(void);
};

#define TEST_LIST const struct test_ test_list_[]
#endif

TEST_LIST = {
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
