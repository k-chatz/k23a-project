#ifndef ML_H
#define ML_H

#include "../include/hash.h"
#include "../include/json_parser.h"

typedef struct ml *ML;

bool ml_create(ML *ml, const char *sw_file, int load);

void ml_destroy(ML *ml);

ulong ml_bow_sz(ML ml);

dictp ml_get_stopwords(ML ml);

void ml_cleanup_sentence(ML ml, char *input);

dictp ml_init_vocabulary_from_json_bow_set(ML ml, setp json_bow_set);

float *ml_bow_json_vector(ML ml, JSON_ENTITY *json, float *bow_vector, int *wc, bool is_user);

void ml_tfidf(ML ml, float *bow_vector, int wc);

void ml_idf_remove(ML ml);

float ml_f1_score(float *y, float *y_pred, int y_size);

void ml_init_vocabulary(ML ml, FILE *fp);

void ml_export_vocabulary(ML ml, char *path);

void ml_print_vocabulary(ML ml, FILE *fp);

void ml_print_vector(ML ml, float *vector);

#endif
