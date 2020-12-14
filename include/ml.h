#ifndef __ML_H__
#define __ML_H__

#include "../include/hash.h"
#include "../include/json_parser.h"

typedef struct ml *ML;

bool ml_create(ML *ht, const char *sw_file);

ulong ml_get_bow_size(ML ml);

dictp ml_bag_of_words(ML ml, char *buf);

void ml_rm_punct_and_upper_case(ML ml, char *input);

bool ml_rm_stop_words(ML ml, char *input);

void ml_rm_digits(ML ml, char *input);

void ml_cleanup(ML ml, char *input);

dictp ml_tokenize_json(ML ml, JSON_ENTITY *json);

float *ml_bow_vector(ML ml, JSON_ENTITY *json);

void tfidf(ML ml);

void print_bow_dict(ML ml);

#endif