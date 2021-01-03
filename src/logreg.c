#include <stdio.h>
#include <assert.h>

#include "../include/logreg.h"

LogReg *lr_new(int weights_len, float learning_rate) {
    unsigned int seed = 12345;
    LogReg *out = malloc(sizeof(*out));
    out->weights_len = weights_len;
    out->weights = malloc(weights_len * sizeof(float));
    for (int i = 0; i < weights_len; i++)
        out->weights[i] = ((float) rand_r(&seed)) / RAND_MAX;
    out->bias = ((float) rand_r(&seed)) / RAND_MAX;
    out->learning_rate = learning_rate;
    return out;
}

LogReg *lr_new_from_file(FILE *fp) {
    int weights_len = 0;
    float bias, learning_rate, *weights = NULL;
    char buf[10];
    fgets(buf, 10, fp);
    learning_rate = atof(buf);
    fgets(buf, 10, fp);
    bias = atof(buf);
    fgets(buf, 10, fp);
    weights_len = atof(buf);
    weights = malloc(weights_len * sizeof(float));
    for (int i = 0; i < weights_len; ++i) {
        fgets(buf, 10, fp);
        weights[i] = atof(buf); //from file
    }

    //create model
    LogReg *model = lr_new(weights_len, learning_rate);
    model->bias = bias;
    model->weights = weights;
    memcpy(model->weights, weights, weights_len);
    return model;
}

void lr_export_model(LogReg *reg, char *path) {
    char filepath[100];
    snprintf(filepath, 100, "%s/%s", path, "model.csv");
    FILE *fp = fopen(filepath, "w+");
    assert(fp != NULL);
    fprintf(fp, "%f\n%f\n%d\n", reg->learning_rate, reg->bias, reg->weights_len);
    for (int i = 0; i < reg->weights_len; ++i) {
        fprintf(fp, "%f\n", reg->weights[i]);
    }
    fclose(fp);
}

void lr_free(LogReg *reg) {
    free(reg->weights);
    free(reg);
}

float lr_sigmoid(float x) { return exp(x) / (1 + exp(x)); }

float lr_loss(float p, bool y) { return -log((y ? p : 1 - p)); }

float lr_predict_one(LogReg *reg, float *X) {
    float lin_sum = 0;
    float p;
    int i;
    for (i = 0; i < reg->weights_len; i++) {
        lin_sum += reg->weights[i] * X[i];
    }
    p = lr_sigmoid(lin_sum + reg->bias);
    return p;
}

float *lr_predict(LogReg *reg, float *Xs, int batch_sz) {
    float *Ps = malloc(sizeof(Ps) * batch_sz);
    for (int i = 0; i < batch_sz; i++)
        Ps[i] = lr_predict_one(reg, &Xs[i * reg->weights_len]);
    return Ps;
}

float lr_train(LogReg *reg, float *Xs, int *Ys, int batch_sz) {
    float *Ps = lr_predict(reg, Xs, batch_sz);

    /* calculate the Deltas */
    float *Deltas = malloc((reg->weights_len + 1) * sizeof(float));
    memset(Deltas, 0, (reg->weights_len + 1) * sizeof(float));
    for (int i = 0; i < batch_sz; i++) {
        int j;
        for (j = 0; j < reg->weights_len; j++) {
            /* j is inner loop for cache efficiency */
            Deltas[j] += reg->learning_rate * (Ps[i] - Ys[i]) * Xs[i * reg->weights_len + j];
        }
        /* Delta for the bias */
        Deltas[j] += reg->learning_rate * (Ps[i] - Ys[i]);
    }

    /* update the weights */
    float max_delta = .0;
    int j;
    for (j = 0; j < reg->weights_len; j++) {
        if (fabsf(Deltas[j]) > max_delta)
            max_delta = fabsf(Deltas[j]);
        reg->weights[j] -= Deltas[j];
    }
    reg->bias -= Deltas[j];

    free(Ps);
    free(Deltas);
    return max_delta;
}
