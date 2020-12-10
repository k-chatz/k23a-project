#ifndef __ML_H__
#define __ML_H__

#include "../include/hash.h"

dictp bag_of_words(char* buf);

void rm_punct_and_upper_case(char *input);

dictp stop_words();

void rm_stop_words(char *input);

#endif