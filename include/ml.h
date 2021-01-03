#ifndef ML_H
#define ML_H

#include "../include/hash.h"
#include "../include/json_parser.h"

typedef struct ml *ML;

bool ml_create(ML *ml, const char *sw_file, int load);

void ml_destroy(ML *ml);

ulong ml_bow_sz(ML ml);

dictp ml_bag_of_words(ML ml, char *buf);

void ml_str_cleanup(ML ml, char *input);

dictp ml_tokenize_json(ML ml, JSON_ENTITY *json);

float *ml_bow_json_vector(ML ml, JSON_ENTITY *json, float *bow_vector, int *wc);

void ml_tfidf(ML ml, float *bow_vector, int wc);

void ml_idf_remove(ML ml);

void ml_set_removed_words_num(ML ml, int c);

int ml_get_removed_words_num(ML ml);

float ml_f1_score(float *y, float *y_pred, int y_size);

void ml_init_vocabulary(ML ml, FILE *fp);

void ml_export_vocabulary(ML ml, char *path);

void ml_print_vocabulary(ML ml, FILE *fp);

void ml_print_vector(ML ml, float *vector);

#endif
