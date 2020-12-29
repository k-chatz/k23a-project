#ifndef __ML_H__
#define __ML_H__

#include "../include/hash.h"
#include "../include/json_parser.h"

typedef struct ml *ML;

bool ml_create(ML *ht, const char *sw_file, int load);

ulong ml_bow_sz(ML ml);

dictp ml_bag_of_words(ML ml, char *buf);

void ml_str_cleanup(ML ml, char *input);

dictp ml_tokenize_json(ML ml, JSON_ENTITY *json);

float *ml_bow_json_vector(ML ml, JSON_ENTITY *json, float *bow_vector, int *wc);

void ml_tfidf(ML ml, float *bow_vector, int wc);

void ml_idf_remove(ML ml);

void set_removed_words_num(ML ml, int c);

int get_removed_words_num(ML ml);

float ml_f1_score(float *y, float* y_pred, int y_size);

void print_bow_dict(ML ml);

void print_vector(ML ml, float *vector);

#endif
