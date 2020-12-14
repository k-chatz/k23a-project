#include "../include/acutest.h"
#include "../include/ml.h"

#ifndef ACUTEST_H

#include <assert.h>

#define TEST_CHECK assert
#define TEST_ASSERT assert
#endif

void remove_punct_and_uppercase(void) {
    //char buffer[128] = "tHe;-. a qui,,,.ck. 255 at 43 a fox.---- j to";
    //rm_punct_and_upper_case(buffer);
    //TEST_CHECK(strcmp(buffer, "the    a qui    ck  255 at 43 a fox      j to") == 0);
}


void remove_stop_words(void) {
    //char buffer1[128] = "the a quick 255 at 43 a fox j f";
    //char buffer2[128] = "the a quick 255 at 43 a fox j at";
    // rm_stop_words(buffer1);
    // rm_stop_words(buffer2);
            // TEST_CHECK(strcmp(buffer1, "quick 255 43 fox j f") == 0);
            // TEST_CHECK(strcmp(buffer2, "quick 255 43 fox j") == 0);
            //TEST_CHECK(1);
}

void remove_digits(void) {
    //char buffer1[128] = "quick 255 43 fox j f";
    //char buffer2[128] = "quick 255 43 fox j";
    //rm_digits(buffer1);
    //rm_digits(buffer2);
            //TEST_CHECK(strcmp(buffer1, "quick   fox j f") == 0);
            //TEST_CHECK(strcmp(buffer2, "quick   fox j") == 0);
}

#ifndef ACUTEST_H

struct test_ {
    const char *name;

    void (*func)(void);
};

#define TEST_LIST const struct test_ test_list_[]
#endif

TEST_LIST = {
        //{"remove_punct_and_uppercase",                                 remove_punct_and_uppercase},
        //{"remove_stop_words",                                          remove_stop_words},
        //{"remove_digits",                                              remove_digits},
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
