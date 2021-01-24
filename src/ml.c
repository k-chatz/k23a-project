#include <stdio.h>
#include <assert.h>
#include <string.h>
#include <ctype.h>
#include <math.h>

#include "../include/colours.h"
#include "../include/hash.h"
#include "../include/json_parser.h"
#include "../include/ml.h"

struct ml {
    dictp vocabulary_bow_dict;
    dictp stopwords;
    int json_ht_load;
};

typedef struct word {
    int position;
    int count;
    float idf;
} Word;

/***Private functions***/

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

dictp ml_stop_words(ML ml, const char *sw_file) {
    char *stop_word, comma;
    assert(ml != NULL);
    FILE *fp = fopen(sw_file, "r");
    assert(fp != NULL);
    /* clang-format off */
    /* @formatter:off */
    dictp sw = dict_config(
            dict_new2(256, 0),
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
    DICT_FOREACH_ENTRY(entry, ml->vocabulary_bow_dict, &iterate_state, ml_bow_sz(ml)) {
        Word *w = (Word *) (entry + ml->vocabulary_bow_dict->htab->key_sz);
        bow_vector[w->position] = bow_vector[w->position] * w->idf;
    }
}

/***Public functions***/

bool ml_create(ML *ml, const char *sw_file, int load) {
    assert(*ml == NULL);
    *ml = (ML) malloc(sizeof(struct ml));
    if ((*ml) != NULL) {
        (*ml)->vocabulary_bow_dict = ml_create_bow_dict();
        dict_config((*ml)->vocabulary_bow_dict,
                    DICT_CONF_CMP, (ht_cmp_func) strncmp,
                    DICT_CONF_KEY_CPY, (ht_key_cpy_func) strncpy,
                    DICT_CONF_HASH_FUNC, djb2_str,
                    DICT_CONF_KEY_SZ_F, str_sz,
                    DICT_CONF_DONE);
        (*ml)->stopwords = NULL;
        if (sw_file) {
            (*ml)->stopwords = ml_stop_words(*ml, sw_file);
            (*ml)->json_ht_load = load;
        }
        return true;
    }
    return false;
}

void ml_destroy(ML *ml) {
    assert(*ml != NULL);
    dict_free((*ml)->vocabulary_bow_dict, NULL);
    if ((*ml)->stopwords != NULL) {
        dict_free((*ml)->stopwords, NULL);
    }
    free(*ml);
}

ulong ml_bow_sz(ML ml) {
    return ml->vocabulary_bow_dict->htab->buf_load;
}

dictp ml_get_stopwords(ML ml) {
    return ml->stopwords;
}

void ml_cleanup_sentence(ML ml, char *input) {
    ml_rm_punct_and_upper_case(ml, input);
    ml_rm_stop_words(ml, input);
    ml_rm_digits(ml, input);
}

dictp ml_init_vocabulary_from_json_bow_set(ML ml, setp json_bow_set) {
    char *entry;
    Word *word = NULL;
    ulong i_start = 0;
    /* Iterate set */
    i_start = 0;
    HSET_FOREACH_ENTRY(entry, json_bow_set, &i_start, json_bow_set->htab->buf_load) {
        word = (Word *) dict_get(ml->vocabulary_bow_dict, entry);
        if (word == NULL) {
            /* does not exist in dict */
            Word w = {ml_bow_sz(ml), 1, 0};
            dict_put(ml->vocabulary_bow_dict, entry, &w);
        } else {
            /* It exists, just up the count */
            word->count++;
        }
        /* erase x from set */
        dict_del(json_bow_set, entry);
    }
    return ml->vocabulary_bow_dict;
}

float *ml_bow_json_vector(ML ml, JSON_ENTITY *json, float *bow_vector, int *wc, bool is_user) {
    StringList *json_keys = json_get_obj_keys(json);
    int capacity = ml_bow_sz(ml);
    memset(bow_vector, 0, capacity * sizeof(float));
    *wc = 0;
    LL_FOREACH(json_key, json_keys) {
        JSON_ENTITY *cur_ent = json_get(json, json_key->data);
        if (cur_ent->type == JSON_STRING) {
            char *sentence = json_to_string(cur_ent);
            char *token = NULL, *rest = NULL;
            tokenizer_t *tok = tokenizer_nlp(sentence);
            while ((token = tokenizer_next(tok)) != NULL) {
                    Word *w = (Word *) dict_get(ml->vocabulary_bow_dict, token);
                    if (w == NULL || token == '\0') {
                        continue;
                    }
                    bow_vector[w->position] += 1.0;
                    (*wc)++;
            }
            tokenizer_free(tok);
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
    int end = ml_bow_sz(ml);
    int c = 0;
    DICT_FOREACH_ENTRY(entry, ml->vocabulary_bow_dict, &iterate_state, end) {
        Word *w = (Word *) (entry + ml->vocabulary_bow_dict->htab->key_sz);
        w->idf = (float) log(ml->json_ht_load / w->count);
        if (w->idf > 9.85 || !strcmp(entry, "\0")) {
            dict_del(ml->vocabulary_bow_dict, entry);
            c++;
            continue;
        }
        w->position = (int) (i - c);
        if (!strcmp(entry, "\0")){
            putchar('\n');
        }
    }
}

float ml_f1_score(float *y, float *y_pred, int y_size) {
    float true_pos = 0.0, true_neg = 0.0, false_pos = 0.0, false_neg = 0.0;
    float precision = 0, recall = 0;

    float *y_pred1 = malloc(y_size * sizeof(float));
    float *y1 = malloc(y_size * sizeof(float));
    for (int i = 0; i < y_size; i++) {
        if (y_pred[i] < 0.5) {
            y_pred1[i] = 0.0;
        } else {
            y_pred1[i] = 1.0;
        }
        if (y[i] == 0) {
            y1[i] = 0.0;
        } else {
            y1[i] = 1.0;
        }
    }


    for (int i = 0; i < y_size; i++) {
        if (y1[i] == 1 && y_pred1[i] == 1) {
            true_pos++;
        } else if (y1[i] == 0 && y_pred1[i] == 0) {
            true_neg++;
        } else if (y1[i] == 1 && y_pred1[i] == 0) {
            false_neg++;
        } else if (y1[i] == 0 && y_pred1[i] == 1) { //else
            false_pos++;
        }
    }

    precision = true_pos / (true_pos + false_pos);
    recall = true_pos / (true_pos + false_neg);


    free(y_pred1);
    free(y1);

    return 2 * precision * recall / (precision + recall);
}

void ml_init_vocabulary(ML ml, FILE *fp) {
    char key[50];
    int pos = 0, count = 0;
    float idf = 0;
    char pos_str[50], count_str[50];
    //skip first row
    fseek(fp, 18, SEEK_SET);
    while (fscanf(fp, "%[^,],%[^,],%[^,],%f\n", key, pos_str, count_str, &idf) != EOF) {
        pos = (int) strtol(pos_str, NULL, 10);
        count = (int) strtol(count_str, NULL, 10);
        Word w = {pos, count, idf};
        dict_put(ml->vocabulary_bow_dict, key, &w);
    }
}

void ml_export_vocabulary(ML ml, char *path) {
    char filepath[100];
    char *entry = NULL;
    ulong i_state = 0;
    snprintf(filepath, 100, "%s/%s", path, "vocabulary.csv");
    FILE *fp = fopen(filepath, "w+");
    assert(fp != NULL);
    fprintf(fp, "key,pos,count,idf\n");
    DICT_FOREACH_ENTRY(entry, ml->vocabulary_bow_dict, &i_state, ml_bow_sz(ml)) {
        Word *w = dict_get(ml->vocabulary_bow_dict, entry);
        fprintf(fp, "%s,%d,%d,%f\n", entry, w->position, w->count, w->idf);
    }
    fclose(fp);
}

void ml_print_vocabulary(ML ml, FILE *fp) {
    char *entry = NULL;
    ulong i_state = 0;
    DICT_FOREACH_ENTRY(entry, ml->vocabulary_bow_dict, &i_state, ml_bow_sz(ml)) {
        Word *w = dict_get(ml->vocabulary_bow_dict, entry);
        fprintf(fp, "%s,%d,%d,%f\n", entry, w->position, w->count, w->idf);
    }
}

void ml_print_vector(ML ml, float *vector) {
    printf("[");
    for (int i = 0; i < ml_bow_sz(ml); i++) {
        if (vector[i]) {
            printf(WARNING"%.3f "RESET, vector[i]);
        } else {
            printf("----- ");
        }
    }
    printf("]\n");
}
