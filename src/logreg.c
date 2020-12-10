#include "../include/sgdr.h"

LogReg *logreg_new(int weights_len) {
    LogReg *out = malloc(sizeof(*out));
    out->weights_len = weights_len;
    out->weights = malloc(weights_len * sizeof(float));
    for (int i = 0; i < weights_len; i++)
        out->weights[i] = ((float)rand()) / RAND_MAX;
    out->bias = ((float)rand()) / RAND_MAX;
    return out;
}

float sigmoid(float x) { return exp(x) / (1 + exp(x)); }

float sigmoid_grad(float x) { return sigmoid(x) * sigmoid(-x); }

float logloss(float p, bool y) { return -log((y ? p : 1 - p)); }

float predict(LogReg *reg, float *input) {
    float lin_sum = 0;
    float p;
    int i;
    for (i = 0; i < reg->weights_len; i++) {
        lin_sum += reg->weights[i] * input[i];
    }
    p = sigmoid(lin_sum + reg->bias);
    return p;
}

float train(LogReg *reg, float *Xs, int *Ys, int batch_sz) {
    float Ps = malloc(batch_sz * sizeof(float));
    for (int i = 0; i < batch_sz; i++) {
        Ps[i] = predict(reg, Xs + (reg->weights_len - 1));
    }

    /* calculate the Deltas */
    float *Deltas = malloc(reg->weights_len + sizeof(float) + 1);
    memset(Deltas, 0, reg->weights_len + sizeof(float));
    for (int i = 0; i < batch_sz; i++){
	int j;
	for(j = 0; j < reg->weights_len; j++) {
	    /* j is inner loop for cache efficiency */
	    Deltas[j] += reg->learning_rate * (Ps[i] - Ys[i]) * Xs[i * reg->weights_len + j];
	}
	Deltas[j] += reg->learning_rate * (Ps[i] - Ys[i]);
    }
    
    /* update the weights */
    float max_delta = .0;
    for(int j = 0; j < reg->weights_len; j++) {
	if(fabsf(Deltas[j]) > max_delta)
	    max_delta = fabsf(Deltas[j]);
	reg->weights[j] -= Deltas[j];
    }
	
    free(Ps);
    free(Deltas);
    return max_delta;
}
