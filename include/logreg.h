#ifndef LOGREG_H
#define LOGREG_H

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

LogReg *lr_new(int weights_len, float learning_rate);

LogReg *lr_new_from_file(FILE *fp, bool *bow);

void lr_cpy(LogReg **dst, LogReg *src);

void lr_export_model(LogReg *reg, bool bow, char * path);

void lr_init_model(LogReg log, FILE *fp);

void lr_free(LogReg *reg);

float lr_sigmoid(float x);

float lr_sigmoid_grad(float x);

float lr_loss(float p, bool y);

float lr_predict_one(LogReg *reg, float *X);

float *lr_predict(LogReg *reg, float *Xs, int batch_sz);

float lr_train(LogReg *reg, float *Xs, int *Ys, int batch_sz);

#endif
