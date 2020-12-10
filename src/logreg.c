#include "../include/sgdr.h"

LogReg *logreg_new(int weights_len) {
    LogReg *out = malloc(sizeof(*out));
    out->weights_len = weights_len;
    out->weights = malloc(weights_len * sizeof(float));
    return out;
}

double sigmoid(double x) {
    return exp(x) / (1 + exp(x));
}

double sigmoid_grad(double x) {
    return sigmoid(x) * sigmoid(-x);
}

double logloss(double p, bool y) {
    return -log((y ? p : 1 - p));
}

double predict(LogReg *reg, double *input) {
    double lin_sum = 0;
    double p;
    int i;
    for(i = 0; i < reg->weights_len-1; i++){
	lin_sum += reg->weights[i] * input[i];
    }
    p = sigmoid(lin_sum + reg->weights[i]);
    return p;
}

double train(LogReg *reg, double *Xs, int Ys, int batch_sz) {
    double Ps = malloc(batch_sz * sizeof(double));
    for(int i = 0; i < batch_sz; i++) {
	Ps[i] = predict(reg, Xs+(reg->weights_len-1))
    }
}
