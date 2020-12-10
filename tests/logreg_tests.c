#include "../include/acutest.h"
#include "../include/logreg.h"


void logreg_test(void) {
    srand(0);
    float Xs[10] =
	{ .5, -.5,
	  .2, .8,
	  1, -1,
	  1.8, -.8,
	  5.0, -2};
    int Ys[5] = {0, 1, 0, 1, 0};
    int batch_sz = 2;
    int batch_num = 2;
    /* overfit; just to see if it works */
    LogReg *reg = logreg_new(2, 0.00005);
    int epochs = 5000;
    for(int i = 0; i < epochs; i++){
	for(int j = 0; j < batch_num; j++) {
	    int batch = rand() % 4;
	    float maxDelta = train(reg, &Xs[2*batch], &Ys[batch], batch_sz);	    
	    TEST_CHECK(maxDelta > 0);
	}
    }

    printf("\nrunning some predictions on our data:\n");
    for(int i = 0; i < 5; i++){
	printf("prediction: %f, actual: %d\n", predict(reg, &Xs[i*2]), Ys[i]);	
    }

}

TEST_LIST = {
    {"logreg" , logreg_test},
    {NULL, NULL}
};
