#include <stdio.h>
#include <assert.h>
#include <string.h>
#include <ctype.h>
#include "../include/hash.h"
#include "../include/json_parser.h"


dictp create_bow_dict() {

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

dictp bag_of_words(dictp bow_dict, char *buf) {
    int position;
    char *token, *rest = NULL;
    for (token = strtok_r(buf, " ", &rest); token != NULL; token = strtok_r(NULL, " ", &rest)) {
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
        *input = (char) tolower(*input);
        if (ispunct((unsigned char) *input)) {
            *input = ' ';
        }
        input++;
    }
    *input = '\0';
}

dictp stop_words() {
    char *stop_word, comma;
    FILE *fp = fopen("resources/unwanted-words.txt", "r");
    assert(fp != NULL);
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
    while (fscanf(fp, "%m[^,]%c", &stop_word, &comma) > 0) {
        dict_put(sw, SET_KEY(stop_word));
        free(stop_word);
    }
    return sw;
}

bool rm_stop_words(char *input) {
    assert(input != NULL);
    int offset = 0;
    char *temp = strdup(input), *token, *rest = NULL;
    char *t = temp;
    strcpy(t, input);
    input[0] = '\0';
    dictp stopwords = stop_words();
    if (stopwords == NULL) {
        return false;
    }
    for (token = strtok_r(t, " ", &rest); token != NULL; token = strtok_r(NULL, " ", &rest)) {
        char *token_val;
        token_val = dict_get(stopwords, token);
        if (token_val == NULL) {
            strcat(input, token);
            strcat(input, " ");
            offset += (int) strlen(token) + 1;
        }
    }
    input[offset - 1] = '\0';
    free(temp);
    return true;
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
    *new = '\0';
}

void ml_cleanup(char *input) {
    rm_punct_and_upper_case(input);
    rm_stop_words(input);
    rm_digits(input);
}

dictp tokenize_json(dictp bow_dict, JSON_ENTITY *json) {
    StringList *json_keys = json_get_obj_keys(json);
    LLFOREACH(json_key, json_keys) {
        JSON_ENTITY *cur_ent = json_get(json, json_key->data);
        if (cur_ent->type == JSON_STRING) {
            char *x = json_to_string(cur_ent);
            // printf("before cleanup: [%s]\n", x);
            ml_cleanup(x);
            bag_of_words(bow_dict, x);
        }
    }
    return bow_dict;
}