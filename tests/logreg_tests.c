#include <stdio.h>
#include <time.h>

#ifdef MAKEFILE
#include "../include/acutest.h"
#endif

#include "../include/logreg.h"

#ifndef ACUTEST_H

#include <assert.h>

#define TEST_CHECK assert
#define TEST_ASSERT assert
#endif

void logreg_test(void) {
    srand(0);
    float Xs[10] =
            {
                    .5, -.5,
                    .2, .8,
                    1, -1,
                    1.8, -.8,
                    5.0, -2
            };

    int Ys[5] = {0, 1, 0, 1, 0};
    int batch_sz = 2;
    int batch_num = 2;

    /* overfit; just to see if it works */
    LogReg *reg = logreg_new(2, 0.0001);
    int epochs = 30000;

    for (int i = 0; i < epochs; i++) {
        for (int j = 0; j < batch_num; j++) {
            int batch = rand() % 4;
            float maxDelta = train(reg, &Xs[2 * batch], &Ys[batch], batch_sz);
            TEST_CHECK(maxDelta > 0);
        }
    }

    printf("\nrunning some predictions on our data:\n");
    float *Ps = predict(reg, Xs, 5);

    for (int i = 0; i < 5; i++) {
        printf("prediction: %f, actual: %d\n", Ps[i], Ys[i]);
    }

    free(Ps);
    logreg_free(reg);
}

#ifndef ACUTEST_H

struct test_ {
    const char *name;

    void (*func)(void);
};


#define TEST_LIST const struct test_ test_list_[]
#endif

TEST_LIST = {
        {"logreg_test", logreg_test},
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
