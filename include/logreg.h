#include <math.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>

typedef
struct {
    int weights_len;
    float *weights;
    float bias;
    float learning_rate;
} LogReg;

LogReg *logreg_new(int weights_len, float learning_rate);
float sigmoid(float x);
float sigmoid_grad(float x);
float logloss(float p, bool y);
float predict(LogReg *reg, float *input);
float train(LogReg *reg, float *Xs, int *Ys, int batch_sz);
