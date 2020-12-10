#include "../include/acutest.h"
#include "../include/ml.h"
#ifndef ACUTEST_H

#include <assert.h>

#define TEST_CHECK assert
#define TEST_ASSERT assert
#endif

void bag_of_words_(void) {
    TEST_CHECK(1);
}

void rm_punctuations_and_replace_lower_case_with_upper_case(void) {
    char buffer[128] = "tHe;-. a qui,,,.ck. 255 at 43 a fox.---- j to";
    rm_punct_and_upper_case(buffer);
    TEST_CHECK(strcmp(buffer, "the a quick 255 at 43 a fox j to") == 0);
}



void rm_stop_words_(void) {
    char buffer[128] = "the a quick 255 at 43 a fox j to";
    rm_stop_words(buffer);
    TEST_CHECK(strcmp(buffer, "quick 255 43 fox j") == 0);
}

void rm_digits_(void) {
    TEST_CHECK(1);
}

#ifndef ACUTEST_H

struct test_ {
    const char *name;

    void (*func)(void);
};

#define TEST_LIST const struct test_ test_list_[]
#endif

TEST_LIST = {
        // {"bag_of_words", bag_of_words},
        // {"rm_punctuations_and_replace_lower_case_with_upper_case", rm_punctuations_and_replace_lower_case_with_upper_case},
        // {"rm_stop_words", rm_stop_words},
        // {"rm_digits", rm_digits},
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
