#include <stdio.h>

#include "../include/acutest.h"
#include "../include/logreg.h"

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
    LogReg *reg = lr_new(2, 0.0001);
    int epochs = 30000;

    for (int i = 0; i < epochs; i++) {
        for (int j = 0; j < batch_num; j++) {
            int batch = rand() % 4;
            float maxDelta = lr_train(reg, &Xs[2 * batch], &Ys[batch], batch_sz);
            TEST_CHECK(maxDelta > 0);
        }
    }

    printf("\nrunning some predictions on our data:\n");
    float *Ps = lr_predict(reg, Xs, 5);

    for (int i = 0; i < 5; i++) {
        printf("prediction: %f, actual: %d\n", Ps[i], Ys[i]);
    }

    free(Ps);
    lr_free(reg);
}

TEST_LIST = {
        {"logreg_test", logreg_test},
        {NULL, NULL}
};
