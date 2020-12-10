#include <stdio.h>
#include <assert.h>
#include <string.h>
#include "../include/hash.h"

dictp bag_of_words(char* buf){
    /* clang-format off */
    /* @formatter:off */
    dictp bow_dict = 
    dict_config( 
        dict_new2(32, sizeof(char*)), 
        DICT_CONF_HASH_FUNC, djb2_str, 
        DICT_CONF_KEY_CPY, strncpy,
        DICT_CONF_CMP, strncmp,
        DICT_CONF_KEY_SZ_F, str_sz, 
        DICT_CONF_DONE
    );
    /* clang-format on */
    /* @formatter:on */
    char* token, *rest = NULL;
    for (token = strtok_r(buf, " ", &rest); token != NULL; token = strtok_r(NULL, " ", &rest))     {
        char *token_word;
        token_word = dict_get(bow_dict, token);
        if (token_word == NULL) {
            char* word = strdup(token);
            dict_put(bow_dict, word, word);
        }
    }

    return bow_dict;
}
 
void rm_punct_and_upper_case(char *input){
    char* old = input, *new = input;
    while(*new){
        *old = tolower(*old);
        if(ispunct((unsigned char)*old)){
            old++;
            *new = *old;
        }
        else{
            *new=*old;
            old++;
            new++;
        }
    }
    new = '\0';
}

dictp stop_words(){

    /* clang-format off */
    /* @formatter:off */
    dictp sw = 
    dict_config( 
        dict_new2(32, 0), 
        DICT_CONF_HASH_FUNC, djb2_str, 
        DICT_CONF_KEY_CPY,(ht_key_cpy_func) strncpy,
        DICT_CONF_CMP, (ht_cmp_func)strncmp,
        DICT_CONF_KEY_SZ_F, str_sz, 
        DICT_CONF_DONE
    );
    /* clang-format on */
    /* @formatter:on */
    int num_put = 0;

#define SET_KEY(X) X, NULL
    dict_putv_distinct(
        sw, &num_put, 
        SET_KEY("the"),
        SET_KEY("to"),  
        SET_KEY("as"), 
        SET_KEY("a"),
        SET_KEY("at"),
        SET_KEY("mm"), 
        NULL
    );
#undef SET_KEY

    return sw;
}

char* rm_stop_words(char *buf,  char *buf2){
    dictp stopwords = stop_words();
    char* token, *rest = NULL;
    for (token = strtok_r(buf, " ", &rest); token != NULL; token = strtok_r(NULL, " ", &rest))     {
        char *token_val;
        token_val = dict_get(stopwords, token);
        if (token_val == NULL) {
            strcat(buf2, token);
            strcat(buf2, " ");
        }
    }
    return buf2;
}

void rm_digits(char *input){
    char* old = input, *new = input;
    while(*new){
        if(isdigit((unsigned char)*old)){
            old++;
            *new = *old;
        }
        else{
            *new=*old;
            old++;
            new++;
        }
    }
    new = '\0';
}

