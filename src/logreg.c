#include <stdio.h>
#include <assert.h>
#include <unistd.h>
#include <pthread.h>

#include "../include/logreg.h"
#include "../include/queue.h"
#include "../include/colours.h"
#include "../include/job_scheduler.h"

extern JobScheduler js;

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

LogReg *lr_new_from_file(FILE *fp, bool *bow) {
    char buf[32];
    LogReg *model = lr_new(0, 0);
    fgets(buf, 32, fp);
    *bow = strcmp(buf, "bow") != 0;
    fgets(buf, 32, fp);
    model->learning_rate = atof(buf);
    fgets(buf, 32, fp);
    model->bias = atof(buf);
    fgets(buf, 32, fp);
    model->weights_len = atof(buf);
    model->weights = malloc(model->weights_len * sizeof(float));
    for (int i = 0; i < model->weights_len; ++i) {
        fgets(buf, 32, fp);
        model->weights[i] = atof(buf);
    }
    return model;
}

void lr_cpy(LogReg **dst, LogReg *src) {
    *dst = lr_new(src->weights_len, src->learning_rate);
    (*dst)->bias = src->bias;
    for (int i = 0; i < (*dst)->weights_len; ++i) {
        (*dst)->weights[i] = src->weights[i];
    }
}

void lr_export_model(LogReg *reg, bool bow, char *path) {
    char filepath[100];
    snprintf(filepath, 100, "%s/%s", path, "model.csv");
    FILE *fp = fopen(filepath, "w+");
    assert(fp != NULL);
    fprintf(fp, "%s\n%f\n%f\n%d\n", bow ? "bow" : "tfidf", reg->learning_rate, reg->bias, reg->weights_len);
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

void *lr_predict_t(Job job) {
    LogReg *reg;
    float *Xs = NULL, *Ps = NULL;
    int i = 0;
    js_get_args(job, &reg, &Xs, &Ps, &i, NULL);
    Ps[i] = lr_predict_one(reg, &Xs[i * reg->weights_len]);
    return NULL;
}

float *lr_predict(LogReg *reg, float *Xs, int batch_sz) {
    float *Ps = malloc(sizeof(Ps) * batch_sz);
    if (js == NULL) {
        for (int i = 0; i < batch_sz; i++) {
            Ps[i] = lr_predict_one(reg, &Xs[i * reg->weights_len]);
        }
    } else {
        Job jobs[batch_sz];
        memset(jobs, 0, sizeof(Job) * batch_sz);
        for (int i = 0; i < batch_sz; i++) {
            jobs[i] = js_create_and_submit_job(js, (void *(*)(void *)) lr_predict_t, false, JOB_ARG(reg), JOB_ARG(Xs),
                                               JOB_ARG(Ps),
                                               JOB_ARG(i), NULL);
        }
        js_execute_all_jobs(js);
        js_wait_all_jobs(js, false);
        for (int i = 0; i < batch_sz; i++) {
            js_destroy_job(&jobs[i]);
        }
    }
    return Ps;
}

void *lr_train_t_(Job job) {
    LogReg *reg;
    float *Xs = NULL, *Ps = NULL, *Deltas = NULL;
    int i = 0, *Ys = NULL;
    js_get_args(job, &reg, &Xs, &Ys, &Ps, &i, NULL);

    //printf(CYAN"Thread [%ld] job %lld i=%d\n"RESET, pthread_self(), js_get_job_id(job), i);

    Deltas = malloc((reg->weights_len + 1) * sizeof(float));
    memset(Deltas, 0, (reg->weights_len + 1) * sizeof(float));

    int j;
    for (j = 0; j < reg->weights_len; j++) {
        /* j is inner loop for cache efficiency */
        Deltas[j] += reg->learning_rate * (Ps[i] - Ys[i]) * Xs[i * reg->weights_len + j];
    }
    /* Delta for the bias */
    Deltas[j] += reg->learning_rate * (Ps[i] - Ys[i]);
    //printf(CYAN"Thread [%ld] job %lld calculated deltas\n"RESET, pthread_self(), js_get_job_id(job));
    return Deltas;
}

float lr_train(LogReg *reg, float *Xs, int *Ys, int batch_sz) {
    float *Ps = lr_predict(reg, Xs, batch_sz);
    /* calculate the Deltas */
    float *Deltas = malloc((reg->weights_len + 1) * sizeof(float));
    memset(Deltas, 0, (reg->weights_len + 1) * sizeof(float));
    if (js == NULL) {
        for (int i = 0; i < batch_sz; i++) {
            int j;
            for (j = 0; j < reg->weights_len; j++) {
                //j is inner loop for cache efficiency
                Deltas[j] += reg->learning_rate * (Ps[i] - Ys[i]) * Xs[i * reg->weights_len + j];
            }
            //Delta for the bias
            Deltas[j] += reg->learning_rate * (Ps[i] - Ys[i]);
        }
    } else {
        //Job jobs[batch_sz];
        Job *jobs = malloc(batch_sz * sizeof(Job));
        memset(jobs, 0, sizeof(Job) * batch_sz);
        for (int i = 0; i < batch_sz; i++) {
            jobs[i] = js_create_and_submit_job(js, (void *(*)(void *)) lr_train_t_, false,
                                               JOB_ARG(reg),
                                               JOB_ARG(Xs),
                                               JOB_ARG(Ys),
                                               JOB_ARG(Ps),
                                               JOB_ARG(i),
                                               NULL);
        }
        js_execute_all_jobs(js);
        js_wait_all_jobs(js, false);
        for (int i = 0; i < batch_sz; i++) {
            for (int j = 0; j < reg->weights_len + 1; j++) {
                Deltas[j] += ((float *) js_get_return_val(js, jobs[i]))[j];
            }
            js_destroy_job(&jobs[i]);
        }
        free(jobs);
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
