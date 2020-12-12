#include <stdio.h>
#include <assert.h>
#include <string.h>
#include <ctype.h>
#include "../include/hash.h"
#include "../include/json_parser.h"



dictp create_bow_dict(){

    /* clang-format off */
    /* @formatter:off */
    dictp bow_dict =
            dict_config(
                    dict_new2(32, sizeof(char *)),
                    DICT_CONF_HASH_FUNC, djb2_str,
                    DICT_CONF_KEY_CPY, strncpy,
                    DICT_CONF_CMP, strncmp,
                    DICT_CONF_KEY_SZ_F, str_sz,
                    DICT_CONF_DONE
            );
    /* clang-format on */
    /* @formatter:on */

    return bow_dict;
}

dictp bag_of_words(dictp bow_dict, char* buf){
    int position;
    char* token, *rest = NULL;
    for (token = strtok_r(buf, " ", &rest); token != NULL; token = strtok_r(NULL, " ", &rest))     {
        char *token_word;
        token_word = dict_get(bow_dict, token);
        position = bow_dict->htab->buf_load;
        if (token_word == NULL) {
            dict_put(bow_dict, token, &position);
        }
    }

    return bow_dict;
}

void rm_punct_and_upper_case(char *input) {
    while (*input) {
        *input = tolower(*input);
        if (ispunct((unsigned char) *input)){
            *input = ' ';
        }
        input++;
    }
    *input = '\0';
}

dictp stop_words() {
    /* clang-format off */
    /* @formatter:off */
    dictp sw =
            dict_config(
                    dict_new2(32, 0),
                    DICT_CONF_HASH_FUNC, djb2_str,
                    DICT_CONF_KEY_CPY, (ht_key_cpy_func) strncpy,
                    DICT_CONF_CMP, (ht_cmp_func) strncmp,
                    DICT_CONF_KEY_SZ_F, str_sz,
                    DICT_CONF_DONE
            );
    /* clang-format on */
    /* @formatter:on */
    int num_put = 0;

#define SET_KEY(X) X, NULL
    
    FILE *fp = fopen("../resources/unwanted-words.txt", "r")
    
    // dict_putv_distinct(
    //         sw, &num_put,
    //         SET_KEY("the"),
    //         SET_KEY("to"),
    //         SET_KEY("as"),
    //         SET_KEY("a"),
    //         SET_KEY("at"),
    //         SET_KEY("mm"),
    //         NULL
    // );
#undef SET_KEY

    return sw;
}

void rm_stop_words(char *input) {
    int offset = 0;
    char *temp = strdup(input), *token, *rest = NULL;
    char *t = temp;
    strcpy(t, input);
    input[0] = '\0';
    dictp stopwords = stop_words();
    for (token = strtok_r(t, " ", &rest); token != NULL; token = strtok_r(NULL, " ", &rest)) {
        char *token_val;
        token_val = dict_get(stopwords, token);
        if (token_val == NULL) {
            strcat(input, token);
            strcat(input, " ");
            offset += strlen(token) + 1;
        }
    }
    input[offset - 1] = '\0';
    free(temp);
}

void rm_digits(char *input) {
    char *old = input, *new = input;
    while (*new) {
        if (isdigit((unsigned char) *old)) {
            old++;
            *new = *old;
        } else {
            *new = *old;
            old++;
            new++;
        }
    }
    new = '\0';
}

void ml_cleanup(char* input){
    rm_punct_and_upper_case(input);
    rm_stop_words(input);
    rm_digits(input);
}

dictp tokenize_json(dictp bow_dict, JSON_ENTITY *json){

    StringList *json_keys = json_get_obj_keys(json);
    LLFOREACH(json_key, json_keys){
        JSON_ENTITY * cur_ent = json_get(json, json_key->data); 
        if (cur_ent->type == JSON_STRING){
            char* x = json_to_string(cur_ent);    
           // printf("before cleanup: [%s]\n", x);
            ml_cleanup(x);
            bag_of_words(bow_dict, x);
        }
    }
    return bow_dict;
}