#include <stdio.h>
#include <assert.h>
#include <string.h>
#include <ctype.h>
#include <math.h>

#include "../include/hash.h"
#include "../include/json_parser.h"
#include "../include/ml.h"

struct ml {
    dictp bow_dict;
    dictp stopwords;
    setp json_set;
    const char *sw_file;
    int json_ht_load;
    int removed_words_num;
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
    if (ml->stopwords == NULL) {
        return false;
    }
    char *temp = strdup(input), *token, *rest = NULL;
    input[0] = '\0';
    for (token = strtok_r(temp, " ", &rest); token != NULL; token = strtok_r(NULL, " ", &rest)) {
        if (dict_get(ml->stopwords, token) == NULL) {
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
        int capacity = ml_bow_sz(ml);
        for (int i = 0; i < capacity; i++) {
            bow_vector[i] = bow_vector[i] * 1 / (float) wc;
        }
    }
}

void ml_idf(ML ml, float *bow_vector) {
    char *entry = NULL;
    ulong iterate_state = 0;
    DICT_FOREACH_ENTRY(entry, ml->bow_dict, &iterate_state, ml->bow_dict->htab->buf_load) {
        Word *w = (Word *) (entry + ml->bow_dict->htab->key_sz);
        bow_vector[w->position] = bow_vector[w->position] * w->idf;
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
        dict_config((*ml)->bow_dict,
                    DICT_CONF_CMP, (ht_cmp_func) strncmp,
                    DICT_CONF_KEY_CPY, (ht_key_cpy_func) strncpy,
                    DICT_CONF_HASH_FUNC, djb2_str,
                    DICT_CONF_KEY_SZ_F, str_sz,
                    DICT_CONF_DONE);
        (*ml)->json_set = ml_create_json_set();
        dict_config((*ml)->json_set,
                    DICT_CONF_CMP, (ht_cmp_func) strncmp,
                    DICT_CONF_KEY_CPY, (ht_key_cpy_func) strncpy,
                    DICT_CONF_HASH_FUNC, djb2_str,
                    DICT_CONF_KEY_SZ_F, str_sz,
                    DICT_CONF_DONE);
        (*ml)->stopwords = ml_stop_words(*ml);
        (*ml)->json_ht_load = load;
        (*ml)->removed_words_num = 0;
        return true;
    }
    return false;
}

void ml_destroy(ML *ml) {
    assert(*ml != NULL);
    dict_free((*ml)->bow_dict, NULL);
    dict_free((*ml)->stopwords, NULL);
    dict_free((*ml)->json_set, NULL);
    free(*ml);
}

ulong ml_bow_sz(ML ml) {
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
            set_put(ml->json_set, token);
        }
    }
    free(buf);
    return ml->bow_dict;
}

dictp ml_tokenize_json(ML ml, JSON_ENTITY *json) {
    char *entry, *x = NULL;
    ulong i_start = 0;
    Word *word = NULL;
    StringList *json_keys = json_get_obj_keys(json);

    /* put distinct words in json_set */
    LL_FOREACH(json_key, json_keys) {
        JSON_ENTITY *cur_ent = json_get(json, json_key->data);
        if (cur_ent->type == JSON_STRING) {
            x = json_to_string(cur_ent);
            ml_str_cleanup(ml, x);
            ml_bag_of_words(ml, x);
        }
    }

    /* Iterate set */
    i_start = 0;
    HSET_FOREACH_ENTRY(entry, ml->json_set, &i_start, ml->json_set->htab->buf_load) {
        word = (Word *) dict_get(ml->bow_dict, entry);
        if (word == NULL) {
            /* does not exist in dict */
            Word w = {0, 1, 0};
            dict_put(ml->bow_dict, entry, &w);
        } else {
            /* It exists, just up the count */
            word->count++;
        }
        /* erase x from set */
        dict_del(ml->json_set, entry);
    }
    return ml->bow_dict;
}

float *ml_bow_json_vector(ML ml, JSON_ENTITY *json, float *bow_vector, int *wc) {
    StringList *json_keys = json_get_obj_keys(json);
    int capacity = ml_bow_sz(ml);
    memset(bow_vector, 0, capacity * sizeof(float));
    *wc = 0;
    LL_FOREACH(json_key, json_keys) {
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
            }
        }
    }
    return bow_vector;
}

void ml_tfidf(ML ml, float *bow_vector, int wc) {
    ml_tf(ml, bow_vector, wc);
    ml_idf(ml, bow_vector);
}

void ml_idf_remove(ML ml) {
    char *entry = NULL;
    ulong iterate_state = 0;
    DICT_FOREACH_ENTRY(entry, ml->bow_dict, &iterate_state, ml->bow_dict->htab->buf_load) {
        Word *w = (Word *) (entry + ml->bow_dict->htab->key_sz);
        w->idf = (float) log(ml->json_ht_load / w->count);
        if (w->idf > 9.85) {
            dict_del(ml->bow_dict, entry);
            continue;
        }
        w->position = (int) i;
    }
}

void set_removed_words_num(ML ml, int c) {
    ml->removed_words_num = c;
}

int get_removed_words_num(ML ml) {
    return ml->removed_words_num;
}

float ml_f1_score(const float *y, const float *y_pred, int y_size) {
    float true_pos = 0.0, true_neg = 0.0, false_pos = 0.0, false_neg = 0.0;
    float precision, recall;
    for (int i = 0; i < y_size; i++) {
        if (y[i] == 1 && y_pred[i] >= 0.5) {
            true_pos++;
        } else if (y[i] == 0 && y_pred[i] < 0.5) {
            true_neg++;
        } else if (y[i] == 1 && y_pred[i] < 0.5) {
            false_neg++;
        } else if (y[i] == 0 && y_pred[i] >= 0.5) { //else
            false_pos++;
        }
    }
    if (((true_pos + false_pos) == 0) || ((true_pos + false_neg) == 0)) return 0;
    precision = true_pos / (true_pos + false_pos);
    recall = true_pos / (true_pos + false_neg);
    return 2 * precision * recall / (precision + recall);
}

void print_bow_dict(ML ml) {
    char *entry = NULL;
    ulong i_state = 0;
    DICT_FOREACH_ENTRY(entry, ml->bow_dict, &i_state, ml->bow_dict->htab->buf_load) {
        printf("[%s]\n", entry);
    }
}

void print_vector(ML ml, float *vector) {
    printf("[");
    for (int i = 0; i < ml_bow_sz(ml); i++) {
        if (vector[i]) {
            printf("%.3f ", vector[i]);
        } else {
            printf("----- ");
        }
    }
    printf("]\n");
}