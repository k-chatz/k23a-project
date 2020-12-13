#include <stdio.h>
#include <assert.h>
#include <string.h>
#include <ctype.h>
#include "../include/hash.h"
#include "../include/json_parser.h"
#include "../include/ml.h"

struct ml {
    dictp bow_dict;
    dictp stop_words;
    const char *sw_file;
};

/***Private functions***/

dictp create_bow_dict() {
    /* clang-format off */
    /* @formatter:off */
    dictp bow_dict = dict_config(
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

dictp ml_stop_words(ML ml) {
    char *stop_word, comma;
    assert(ml != NULL);
    FILE *fp = fopen(ml->sw_file, "r"); 
    assert(fp != NULL);
    /* clang-format off */
    /* @formatter:off */
    dictp sw = dict_config(
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
    fclose(fp);
    return sw;
}

/***Public functions***/

bool ml_create(ML *ml, const char *sw_file) {
    assert(*ml == NULL);
    assert(sw_file != NULL);
    *ml = (ML) malloc(sizeof(struct ml));
    if ((*ml) != NULL) {
        (*ml)->sw_file = sw_file;
        (*ml)->bow_dict = create_bow_dict();
        (*ml)->stop_words = ml_stop_words(*ml); 
        return true;
    }
    return false;
}

ulong ml_get_bow_size(ML ml){
    return ml->bow_dict->htab->buf_load;
}

dictp ml_bag_of_words(ML ml, char *buf) {
    int position;
    char *token, *rest = NULL;
    for (token = strtok_r(buf, " ", &rest); token != NULL; token = strtok_r(NULL, " ", &rest)) {
        char *token_word;
        token_word = dict_get(ml->bow_dict, token);
        position = ml->bow_dict->htab->buf_load;
        if (token_word == NULL) {
            int token_len = strlen(token);
            if (token_len <= 3 || token_len > 7){
                continue;
            }
            dict_put(ml->bow_dict, token, &position);
        }
    }

    return ml->bow_dict;
}

void ml_rm_punct_and_upper_case(ML ml, char *input) {
    while (*input) {
        *input = (char) tolower(*input);
        if (ispunct((unsigned char) *input)) {
            *input = ' ';
        }
        input++;
    }
    *input = '\0';
}


bool ml_rm_stop_words(ML ml, char *input) {
    assert(input != NULL);
    int offset = 0;
    char *temp = strdup(input), *token, *rest = NULL;
    char *t = temp;
    strcpy(t, input);
    input[0] = '\0';
    if (ml->stop_words == NULL) {
        return false;
    }
    for (token = strtok_r(t, " ", &rest); token != NULL; token = strtok_r(NULL, " ", &rest)) {
        char *token_val;
        token_val = dict_get(ml->stop_words, token);
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

void ml_rm_digits(ML ml, char *input) {
    while (*input) {
        if (isdigit((unsigned char) *input)) {
            *input = ' ';
        }
        input++;
    }
    *input = '\0';
}

void ml_cleanup(ML ml, char *input) {
    ml_rm_punct_and_upper_case(ml, input);
    ml_rm_stop_words(ml, input);
    ml_rm_digits(ml, input);
}

dictp ml_tokenize_json(ML ml, JSON_ENTITY *json) {
    StringList *json_keys = json_get_obj_keys(json);
    LLFOREACH(json_key, json_keys) {
        JSON_ENTITY *cur_ent = json_get(json, json_key->data);
        if (cur_ent->type == JSON_STRING) {
            char *x = json_to_string(cur_ent);
            // printf("before cleanup: [%s]\n", x);
            ml_cleanup(ml, x);
            ml_bag_of_words(ml, x);
        }
    }
    return ml->bow_dict;
}

float *ml_bow_vector(ML ml, JSON_ENTITY *json) {
    StringList *json_keys = json_get_obj_keys(json);
    int capacity = ml->bow_dict->htab->buf_load;
    float *bow_vector = malloc(capacity * sizeof(float));
    memset(bow_vector, 0.0, capacity);
    LLFOREACH(json_key, json_keys) {
        JSON_ENTITY *cur_ent = json_get(json, json_key->data);
        if (cur_ent->type == JSON_STRING) {
            char *x = json_to_string(cur_ent);
            char *token = NULL, *rest = NULL;
            for (token = strtok_r(x, " ", &rest); token != NULL; token = strtok_r(NULL, " ", &rest)) {
                int *token_val;
                token_val = dict_get(ml->bow_dict, token);
                if (*(int *) token_val >= 0) {
                    bow_vector[*(int *) token_val] += 1.0;
                }
            }
        }
    }
    return bow_vector;
}

void print_bow_dict(ML ml){
    char *x = NULL;
    while ((x = (char *) dict_iterate(ml->bow_dict))) {
        valp value = dict_get(ml->bow_dict, x);
        printf("%s\n", x);
    }
}
