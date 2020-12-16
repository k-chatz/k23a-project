#include <stdio.h>
#include <assert.h>
#include <string.h>
#include <ctype.h>
#include <math.h>

#include "../include/hash.h"
#include "../include/json_parser.h"
#include "../include/ml.h"
#include "../include/hset.h"

struct ml {
    dictp bow_dict;
    dictp stop_words;
    setp json_set;
    const char *sw_file;
    int json_ht_load;
};

typedef struct word {
    int position;
    int count;
    float idf;
} Word;

/***Private functions***/

setp ml_create_json_set() {
    return set_new(10);
}

dictp ml_create_bow_dict() {
    /* clang-format off */
    /* @formatter:off */
    dictp bow_dict = dict_config(
            dict_new2(10, sizeof(Word)),
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
    if (ml->stop_words == NULL) {
        return false;
    }
    char *token_val;
    char *temp = strdup(input), *token, *rest = NULL;
    input[0] = '\0';
    for (token = strtok_r(temp, " ", &rest); token != NULL; token = strtok_r(NULL, " ", &rest)) {
        token_val = dict_get(ml->stop_words, token);
        if (token_val == NULL) {
            strcat(input, token);
            strcat(input, " ");
        }
    }
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

void ml_tf(ML ml, float *bow_vector, int wc) {
    if (wc != 0) {
        int capacity = ml_get_bow_size(ml);
        for (int i = 0; i < capacity; i++) {
            bow_vector[i] = bow_vector[i] * 1 / (float) wc;
        }
    }
}

void ml_idf(ML ml, float *bow_vector) {
    char *entry = NULL;
    ulong iterate_state = 0;
    DICT_FOREACH_ENTRY(entry, ml->bow_dict, &iterate_state) {
        Word *w = (Word *) (entry + ml->bow_dict->htab->key_sz);
        w->idf = (float) log(ml->json_ht_load / w->count);
        //printf("%s\tcount:[%d], position:[%d], ml_idf:[%f]\n", entry, w->count, w->position, w->idf);
        //for (int i = 0; i < ml_get_bow_size(ml); i++) {
            //bow_vector[i] = bow_vector[i] * w->idf;
            bow_vector[w->position] = bow_vector[w->position] * w->idf;
        //}
    }
}

/***Public functions***/

bool ml_create(ML *ml, const char *sw_file, int load) {
    assert(*ml == NULL);
    assert(sw_file != NULL);
    *ml = (ML) malloc(sizeof(struct ml));
    if ((*ml) != NULL) {
        (*ml)->sw_file = sw_file;
        (*ml)->bow_dict = ml_create_bow_dict();
        (*ml)->json_set = ml_create_json_set();
        (*ml)->stop_words = ml_stop_words(*ml);
        (*ml)->json_ht_load = load;
        return true;
    }
    return false;
}

ulong ml_get_bow_size(ML ml) {
    return ml->bow_dict->htab->buf_load;
}

void ml_str_cleanup(ML ml, char *input) {
    ml_rm_punct_and_upper_case(ml, input);
    ml_rm_stop_words(ml, input);
    ml_rm_digits(ml, input);
}

dictp ml_bag_of_words(ML ml, char *input) {
    char *token, *rest = NULL;
    char *buf = strdup(input);
    for (token = strtok_r(buf, " ", &rest); token != NULL; token = strtok_r(NULL, " ", &rest)) {
        if (!set_in(ml->json_set, token)) {
            size_t token_len = strlen(token);
            if (token_len <= 3 || token_len > 7) {
                continue;
            }
            set_put(ml->json_set, token);
        }
    }
    free(buf);
    return ml->bow_dict;
}

dictp ml_tokenize_json(ML ml, JSON_ENTITY *json) {
    StringList *json_keys = json_get_obj_keys(json);
    LLFOREACH(json_key, json_keys) {
        JSON_ENTITY *cur_ent = json_get(json, json_key->data);
        if (cur_ent->type == JSON_STRING) {
            char *x = json_to_string(cur_ent);
            ml_str_cleanup(ml, x);
            ml_bag_of_words(ml, x);
        }
    }
    //check if contents of json set exist in bow_dict
    // if one does, count++,
    // else put it and count++
    /*Iterate set*/
    keyp x = NULL;
    Word *word = NULL;
    while ((x = set_iterate(ml->json_set))) {
        word = (Word *) dict_get(ml->bow_dict, (char *) x);
        /* does not exist in dict */
        if (word == NULL) {
            Word w = {ml_get_bow_size(ml), 1, 0};

            dict_put(ml->bow_dict, x, &w);
            /*It exists, just up the count*/
        } else {
            word->count++;
        }
        //now, we gotta erase everything in json_set to use it for the next json
        /*erase x from set*/
        dict_del(ml->json_set, x);
    }
    return ml->bow_dict;
}

float *ml_bow_json_vector(ML ml, JSON_ENTITY *json, int *wc) {
    StringList *json_keys = json_get_obj_keys(json);
    int capacity = ml_get_bow_size(ml);
    float *bow_vector = malloc(capacity * sizeof(float));
    memset(bow_vector, 0, capacity * sizeof(float));
    *wc = 0;
    LLFOREACH(json_key, json_keys) {
        JSON_ENTITY *cur_ent = json_get(json, json_key->data);
        if (cur_ent->type == JSON_STRING) {
            char *value = json_to_string(cur_ent);
            char *token = NULL, *rest = NULL;
            for (token = strtok_r(value, " ", &rest); token != NULL; token = strtok_r(NULL, " ", &rest)) {
                Word *w = (Word *) dict_get(ml->bow_dict, token);
                if (w == NULL) {
                    continue;
                }
                bow_vector[w->position] += 1.0;
                (*wc)++;
                // if (bow_vector[w->position] == 1) {
                //     w->count++;
                // }
            }
        }
    }
    return bow_vector;
}

void ml_tfidf(ML ml, float *bow_vector, int wc) {
    ml_tf(ml, bow_vector, wc);
    ml_idf(ml, bow_vector);
    //TODO: Calculate bow_vector
    // kathe stoixeio tou pinaka tha to pollaplasiazeis me to idf toy
    // bow_vector[i] = bow_vector[i] * w->idf;
    print_vector(ml, bow_vector);
}

void print_bow_dict(ML ml) {
    char *x = NULL;
    while ((x = (char *) dict_iterate(ml->bow_dict))) {
        printf("%s\n", x);
    }
}

void print_vector(ML ml, float *vector) {
    printf("[");
    for (int i = 0; i < ml_get_bow_size(ml); i++) {
        if (vector[i]) {
            printf("%.3f ", vector[i]);
        } else {
            printf("----- ");
        }
    }
    printf("]\n");
}
