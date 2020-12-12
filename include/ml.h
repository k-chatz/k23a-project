#ifndef __ML_H__
#define __ML_H__

#include "../include/hash.h"
#include "../include/json_parser.h"

dictp create_bow_dict();

dictp bag_of_words(char *buf);

void rm_punct_and_upper_case(char *input);

dictp stop_words();

void rm_stop_words(char *input);

void rm_digits(char *input);

void ml_cleanup(char* input);

dictp tokenize_json(dictp bow_dict, JSON_ENTITY *json);

#endif