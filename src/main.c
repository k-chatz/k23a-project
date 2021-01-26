#include <stdio.h>
#include <fcntl.h>
#include <dirent.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>

#include "../include/colours.h"
#include "../include/lists.h"
#include "../include/spec_to_specs.h"
#include "../include/json_parser.h"
#include "../include/ml.h"
#include "../include/logreg.h"
#include "../include/unique_rand.h"
#include "../include/job_scheduler.h"

#define epochs 5
#define batch_size 2000
#define learning_rate 0.0001
#define THREADS 5000

pthread_mutex_t wc_lock;

typedef struct options {
    char *dataset_dir;
    char *labelled_dataset_path;
    char *stop_words_path;
    char *vec_mode;
    char *export_path;
} Options;

JobScheduler js = NULL;

void read_options(int argc, char **argv, Options *o) {
    int i;
    char *opt, *optVal;
    for (i = 1; i < argc; ++i) {
        opt = argv[i];
        optVal = argv[i + 1];
        if (strcmp(opt, "-dir") == 0) {
            if (optVal != NULL && optVal[0] != '-') {
                o->dataset_dir = optVal;
            }
        } else if (strcmp(opt, "-csv") == 0) {
            if (optVal != NULL && optVal[0] != '-') {
                o->labelled_dataset_path = optVal;
            }
        } else if (strcmp(opt, "-sw") == 0) {
            if (optVal != NULL && optVal[0] != '-') {
                o->stop_words_path = optVal;
            }
        } else if (strcmp(opt, "-m") == 0) {
            if (optVal != NULL && optVal[0] != '-') {
                o->vec_mode = optVal;
            }
        } else if (strcmp(opt, "-ex") == 0) {
            if (optVal != NULL && optVal[0] != '-') {
                o->export_path = optVal;
            }
        }
    }
}

void free_json_ht_ent(JSON_ENTITY **val) {
    json_entity_free(*val);
}

void read_labelled_dataset_csv(STS *dataset_X, char *labelled_dataset, char *flag) {
    char left_spec_id[50], right_spec_id[50], label[50];
    FILE *fp = fopen(labelled_dataset, "r");
    //skip first row
    uint merges = 0, diffs = 0;
    fseek(fp, 33, SEEK_SET);
    while (fscanf(fp, "%[^,],%[^,],%s\n", left_spec_id, right_spec_id, label) != EOF) {
        //fprintf(stdout, "%s, %s\n", left_spec_id, right_spec_id);
        if (!strcmp(label, flag) && strcmp(left_spec_id, right_spec_id) != 0) {
            if (!strncmp(flag, "1", 1)) {
                merges += sts_merge(dataset_X, left_spec_id, right_spec_id) >= 0;
            } else {
                diffs += sts_diff(dataset_X, left_spec_id, right_spec_id) >= 0;
            }
        }
    }
    fclose(fp);
}

void read_user_labelled_dataset_csv(char *user_labelled_dataset_file, Pair **pairs, int *counter) {
    assert(user_labelled_dataset_file != NULL);
    assert(*pairs == NULL);
    assert(*counter == 0);
    FILE *fp = fopen(user_labelled_dataset_file, "r");
    char left_spec_id[50], right_spec_id[50];
    //skip first row
    fseek(fp, 27, SEEK_SET);
    while (fscanf(fp, "%[^,],%s\n", left_spec_id, right_spec_id) != EOF) {
        (*pairs) = realloc(*pairs, (*counter + 1) * sizeof(struct pair));
        assert(*pairs != NULL);
        //fprintf(stdout, "Save to pairs array: %s, %s\n", left_spec_id, right_spec_id);
        (*pairs)[*counter].spec1 = strdup(left_spec_id);
        (*pairs)[*counter].spec2 = strdup(right_spec_id);
        (*pairs)[*counter].relation = -1;
        (*pairs)[*counter].type = NAT;
        (*counter)++;
    }
    fclose(fp);
}

STS *init_sts_dataset_X(char *path) {
    struct dirent **name_list = NULL, **internals_name_list = NULL;
    int n, internals;
    char new_path[512], spec_name[1024];
    STS *sts = sts_new();

    /* scan external dataset_dir*/
    n = scandir(path, &name_list, NULL, alphasort);
    if (n == -1) {
        perror("scandir");
        exit(EXIT_FAILURE);
    }
    while (n--) {
        if (!strcmp(name_list[n]->d_name, ".") || !strcmp(name_list[n]->d_name, "..")) {
            free(name_list[n]);
            continue;
        }
        snprintf(new_path, 512, "%s/%s", path, name_list[n]->d_name);
        /* for each site collect specs*/
        internals = scandir(new_path, &internals_name_list, NULL, alphasort);
        if (internals == -1) {
            perror("scandir");
            exit(EXIT_FAILURE);
        }
        while (internals--) {
            if (!strcmp(internals_name_list[internals]->d_name, ".") ||
                !strcmp(internals_name_list[internals]->d_name, "..")) {
                free(internals_name_list[internals]);
                continue;
            }
            internals_name_list[internals]->d_name[strlen(internals_name_list[internals]->d_name) - 5] = '\0';
            snprintf(spec_name, 1024, "%s//%s", name_list[n]->d_name, internals_name_list[internals]->d_name);
            // printf("file name: %s\n", spec_name);
            sts_add(sts, spec_name);
            free(internals_name_list[internals]);
        }
        free(internals_name_list);
        free(name_list[n]);
    }

    free(name_list);
    return sts;
}

dictp user_json_dict(char *path) {
    struct dirent **name_list = NULL, **internals_name_list = NULL;
    int n, internals;
    char new_path[512], spec_name[1024];
    char json_name[512], json_path[1024];
    dictp json_dict = dict_new2(128, sizeof(JSON_ENTITY *));
    dict_config(json_dict,
                DICT_CONF_HASH_FUNC, djb2_str,
                DICT_CONF_KEY_CPY, strncpy,
                DICT_CONF_CMP, strncmp,
                DICT_CONF_KEY_SZ_F, str_sz,
                DICT_CONF_DONE
    );

    /* scan external dataset_dir*/
    n = scandir(path, &name_list, NULL, alphasort);
    if (n == -1) {
        perror("scandir");
        exit(EXIT_FAILURE);
    }
    while (n--) {
        if (!strcmp(name_list[n]->d_name, ".") || !strcmp(name_list[n]->d_name, "..")) {
            free(name_list[n]);
            continue;
        }
        snprintf(new_path, 512, "%s/%s", path, name_list[n]->d_name);
        /* for each site collect specs*/
        internals = scandir(new_path, &internals_name_list, NULL, alphasort);
        if (internals == -1) {
            perror("scandir");
            exit(EXIT_FAILURE);
        }
        while (internals--) {
            if (!strcmp(internals_name_list[internals]->d_name, ".") ||
                !strcmp(internals_name_list[internals]->d_name, "..")) {
                free(internals_name_list[internals]);
                continue;
            }
            strcpy(json_name, internals_name_list[internals]->d_name);
            snprintf(json_path, 1025, "%s/%s", new_path, json_name);
            internals_name_list[internals]->d_name[strlen(internals_name_list[internals]->d_name) - 5] = '\0';
            snprintf(spec_name, 1024, "%s//%s", name_list[n]->d_name, internals_name_list[internals]->d_name);
            /* Opening, parsing and creating a JSON ENTITY object for the 'json_path' file*/
            JSON_ENTITY *ent = json_parse_file(json_path);
            /* Putting this JSON_ENTITY in 'json_dict' hashtable*/
            dict_put(json_dict, spec_name, &ent);
            free(internals_name_list[internals]);
        }
        free(internals_name_list);
        free(name_list[n]);
    }
    free(name_list);
    return json_dict;
}

float *vec_from_json(ML ml, dictp json_dict, char *spec, bool tfidf) {
    int wc = 0;
    float *vector = malloc(ml_bow_sz(ml) * sizeof(float));
//    memset(vector, 0, ml_bow_sz(ml) * sizeof(float));
    JSON_ENTITY **json = (JSON_ENTITY **) dict_get(json_dict, spec);
    ml_bow_json_vector(ml, *json, vector, &wc, false);
    if (tfidf) {
        ml_tfidf(ml, vector, wc);
    }

    return vector;
}

#define LOCK_ pthread_mutex_lock(&mtx)
#define UNLOCK_ pthread_mutex_unlock(&mtx)

pthread_mutex_t mtx = PTHREAD_MUTEX_INITIALIZER;

void *fill_vector(Job job) {
    ML ml = NULL;
    dictp json_dict = NULL, vectors_dict = NULL;
    Pair **pairs = NULL;
    float *result_vector = NULL;
    int *y, x = 0, start = 0, end = 0;
    bool random;
    URand ur = NULL;
    js_get_args(job, &ml, &json_dict, &vectors_dict, &pairs, &result_vector, &y, &start, &end, &ur, &random, NULL);
    float *bow_vector_1 = NULL, *bow_vector_2 = NULL;
    for (int i = start; i < end; i++) {
        x = random ? ur_get(ur) : i;
        bow_vector_1 = dict_get(vectors_dict, (*pairs)[x].spec1);
        bow_vector_2 = dict_get(vectors_dict, (*pairs)[x].spec2);
        for (int c = 0; c < ml_bow_sz(ml); c++) {
            result_vector[i * ml_bow_sz(ml) + c] = fabs((bow_vector_1[c] - bow_vector_2[c]));
        }
        y[i] = (*pairs)[x].relation;
    }
    return NULL;
}

void
prepare_set(int start, int end, bool random, URand ur, STS *X, ML ml,
            dictp json_dict, dictp vectors_dict, Pair **pairs, float *result_vector, int *y, bool tfidf, bool is_user) {
    int wc = 0, x = 0;
    JSON_ENTITY **json1 = NULL, **json2 = NULL;
    SpecEntry *spec1 = NULL, *spec2 = NULL;
    if (js) {

        uint threads = js_get_execution_threads(js);

        Job *jobs = malloc(threads * sizeof(Job));
        //memset(jobs, 0, threads  * sizeof(Job));
        int offset = end / threads, start_t = 0, end_t = 0;
        int i = 0;
        for (i = 0; i < threads; i++) {
            start_t = i * offset;
            end_t = (i + 1) * offset;
            jobs[i] = NULL;
            js_create_job(&jobs[i], (void *(*)(void *)) fill_vector, JOB_ARG(ml), JOB_ARG(json_dict),
                          JOB_ARG(vectors_dict), JOB_ARG(pairs), JOB_ARG(result_vector),
                          JOB_ARG(y), JOB_ARG(start_t), JOB_ARG(end_t), JOB_ARG(ur), JOB_ARG(random), NULL);
            js_submit_job(js, jobs[i]);
        }
        js_execute_all_jobs(js);
        js_wait_all_jobs(js, false);

        for (int i = 0; i < threads; i++) {
            //js_destroy_job(&jobs[i]);
        }

        if ((end - end_t) > 0) {
            Job last_job = NULL;
            start_t = end_t;
            end_t = end;
            last_job = NULL;
            js_create_job(&last_job, (void *(*)(void *)) fill_vector, JOB_ARG(ml), JOB_ARG(json_dict),
                          JOB_ARG(vectors_dict), JOB_ARG(pairs), JOB_ARG(result_vector),
                          JOB_ARG(y), JOB_ARG(start_t),
                          JOB_ARG(end_t), JOB_ARG(ur), JOB_ARG(random), NULL);
            js_submit_job(js, last_job);

            js_execute_all_jobs(js);
            js_wait_all_jobs(js, false);
            //js_destroy_job(&last_job);
        }

        free(jobs);
    } else {
        for (int i = start; i < end; i++) {
            float *bow_vector_1 = NULL, *bow_vector_2 = NULL;
            x = random ? ur_get(ur) : i;
            bow_vector_1 = dict_get(vectors_dict, (*pairs)[x].spec1);
            bow_vector_2 = dict_get(vectors_dict, (*pairs)[x].spec2);
            for (int c = 0; c < ml_bow_sz(ml); c++) {
                result_vector[(i - start) * ml_bow_sz(ml) + c] = fabs((bow_vector_1[c] - bow_vector_2[c]));
            }
            y[i - start] = (*pairs)[x].relation;
        }
    }


}

Pair *shuffle_dataset(Pair *pairs, int dataset_size) {
    URand ur = NULL;
    Pair *shuffle_pairs = malloc(dataset_size * sizeof(struct pair));
    int x = 0;
    ur_create(&ur, 0, dataset_size - 1);
    for (int i = 0; i < dataset_size; i++) {
        x = ur_get(ur);
        shuffle_pairs[i] = pairs[x];
    }
    return shuffle_pairs;
}

float calc_max_loss(float *losses, float *y_pred, int *y, int offset) {
    float max_loss = 0;
    /* Calculate max_loss */
    for (int i = 0; i < offset; i++) {
        losses[i] = lr_loss(y_pred[i], y[i]);
        if (i == 0) {
            max_loss = losses[i];
        } else {
            if (max_loss < losses[i]) {
                max_loss = losses[i];
            }
        }
    }
    return max_loss;
}

float calc_med_loss(float *losses, float *y_pred, int *y, int offset) {
    float med_loss = 0, sum = 0;
    /* Calculate max_loss */
    for (int i = 0; i < offset; i++) {
        losses[i] = lr_loss(y_pred[i], y[i]);
        sum += losses[i];
    }
    med_loss = sum / offset;
    return med_loss;
}

void merge_set(Pair *set, int set_sz, int similar_set_sz, Pair *similar_pairs_train,
               Pair *different_pairs_train) {
    URand ur = NULL;
    int rand_i = 0;
    ur_create(&ur, 0, set_sz - 1);
    for (int i = 0; i < set_sz; i++) {
        rand_i = ur_get(ur);
        if (i < similar_set_sz) {
            set[rand_i] = similar_pairs_train[i];
        } else {
            set[rand_i] = different_pairs_train[i - similar_set_sz];
        }
    }
    ur_destroy(&ur);
}

void split(Pair *similar_pairs, Pair *different_pairs, int similar_sz, int different_sz,
           int *train_sz, Pair **train_set,
           int *test_sz, Pair **test_set,
           int *val_sz, Pair **val_set) {
    Pair *similar_pairs_train = NULL,
            *similar_pairs_test = NULL,
            *similar_pairs_val = NULL,
            *different_pairs_train = NULL,
            *different_pairs_test = NULL,
            *different_pairs_val = NULL;
    int similar_train_set_sz = 0, similar_test_set_sz = 0, similar_val_set_sz = 0, different_train_set_sz = 0,
            different_test_set_sz = 0, different_val_set_sz = 0;

    /* calculate similar set sizes */
    similar_train_set_sz = (similar_sz / 2) % 2 ? (int) (similar_sz / 2 - 1) : (int) (similar_sz / 2);
    similar_test_set_sz = (similar_sz - similar_train_set_sz) / 2;
    similar_val_set_sz = similar_test_set_sz;

    similar_pairs_train = malloc(similar_train_set_sz * sizeof(Pair));
    similar_pairs_test = malloc(similar_test_set_sz * sizeof(Pair));
    similar_pairs_val = malloc(similar_val_set_sz * sizeof(Pair));

    memcpy(similar_pairs_train, similar_pairs, similar_train_set_sz * sizeof(Pair));
//    memcpy(similar_pairs_train + similar_train_set_sz, similar_pairs, similar_train_set_sz * sizeof(Pair));
    memcpy(similar_pairs_test, similar_pairs + similar_train_set_sz, similar_test_set_sz * sizeof(Pair));
    memcpy(similar_pairs_val, similar_pairs + similar_train_set_sz + similar_test_set_sz,
           similar_val_set_sz * sizeof(Pair));

    /* calculate different set sizes */
    different_train_set_sz = (different_sz / 2) % 2 ? (int) (different_sz / 2 - 1) : (int) (different_sz / 2);
    different_test_set_sz = (different_sz - different_train_set_sz) / 2;
    different_val_set_sz = different_test_set_sz;

    different_pairs_train = malloc(different_train_set_sz * sizeof(Pair));
    different_pairs_test = malloc(different_test_set_sz * sizeof(Pair));
    different_pairs_val = malloc(different_val_set_sz * sizeof(Pair));

    memcpy(different_pairs_train, different_pairs, different_train_set_sz * sizeof(Pair));
    memcpy(different_pairs_test, different_pairs + different_train_set_sz, different_test_set_sz * sizeof(Pair));
    memcpy(different_pairs_val, different_pairs + different_train_set_sz + different_test_set_sz,
           different_val_set_sz * sizeof(Pair));

    /*create the whole train set, test set and val set*/
    *train_sz = similar_train_set_sz + different_train_set_sz;
    *test_sz = similar_test_set_sz + different_test_set_sz;
    *val_sz = similar_val_set_sz + different_val_set_sz;

    *train_set = malloc(*train_sz * sizeof(Pair));
    *test_set = malloc(*test_sz * sizeof(Pair));
    *val_set = malloc(*val_sz * sizeof(Pair));

    /* merge the train, test & val sets */
    merge_set(*train_set, *train_sz, similar_train_set_sz, similar_pairs_train, different_pairs_train);
    merge_set(*test_set, *test_sz, similar_test_set_sz, similar_pairs_train, different_pairs_train);
    merge_set(*val_set, *val_sz, similar_val_set_sz, similar_pairs_train, different_pairs_train);

    free(different_pairs_train);
    free(different_pairs_test);
    free(different_pairs_val);
    free(similar_pairs_train);
    free(similar_pairs_test);
    free(similar_pairs_val);
}

void init_json_dict(STS *X, dictp *json_dict, char *dataset_dir) {
    ulong i_state = 0;
    char json_path[280], json_website[128], json_num[128], *entry = NULL;

    /* create json hashtable*/
    *json_dict = dict_new3(128, sizeof(JSON_ENTITY *), X->dict->htab->buf_cap);
    dict_config(*json_dict,
                DICT_CONF_HASH_FUNC, djb2_str,
                DICT_CONF_KEY_CPY, strncpy,
                DICT_CONF_CMP, strncmp,
                DICT_CONF_KEY_SZ_F, str_sz,
                DICT_CONF_DONE
    );

    /* create json hash table using dataset X */
    DICT_FOREACH_ENTRY(entry, X->dict, &i_state, X->dict->htab->buf_load) {

        /* Constructing the key for this JSON_ENTITY entry*/
        sscanf(entry, "%[^/]//%[^/]", json_website, json_num);

        /* Constructing the path*/
        snprintf(json_path, 280, "%s/%s/%s.json", dataset_dir, json_website, json_num);

        /* Opening, parsing and creating a JSON ENTITY object for the 'json_path' file*/
        JSON_ENTITY *ent = json_parse_file(json_path);

        /* Putting this JSON_ENTITY in 'json_dict' hashtable*/
        dict_put(*json_dict, entry, &ent);
    }
}

void init_json_train_set(setp *json_train_keys, int train_sz, Pair *train_set) {
    *json_train_keys = set_new(128);
    dict_config(*json_train_keys,
                DICT_CONF_CMP, (ht_cmp_func) strncmp,
                DICT_CONF_KEY_CPY, (ht_key_cpy_func) strncpy,
                DICT_CONF_HASH_FUNC, djb2_str,
                DICT_CONF_KEY_SZ_F, str_sz,
                DICT_CONF_DONE
    );

    /*Add jsons to vocab */
    for (int i = 0; i < train_sz; i++) {
        set_put(*json_train_keys, train_set[i].spec1);
        set_put(*json_train_keys, train_set[i].spec2);
    }
}

void tokenize_json_train_set(ML ml, setp train_json_files_set, dictp json_dict) {
    ulong i_state = 0;
    char *entry = NULL, *sentence = NULL, *token = NULL, *rest = NULL;
    JSON_ENTITY **json = NULL;
    StringList *json_keys = NULL;
    setp json_bow_set;
    json_bow_set = set_new(10);
    dict_config(json_bow_set,
                DICT_CONF_CMP, (ht_cmp_func) strncmp,
                DICT_CONF_KEY_CPY, (ht_key_cpy_func) strncpy,
                DICT_CONF_HASH_FUNC, djb2_str,
                DICT_CONF_KEY_SZ_F, str_sz,
                DICT_CONF_DONE);

    /* foreach json file */
    HSET_FOREACH_ENTRY(entry, train_json_files_set, &i_state, train_json_files_set->htab->buf_load) {
        json = (JSON_ENTITY **) dict_get(json_dict, entry);
        if (json != NULL) {

            /* put distinct words in json_set */
            json_keys = json_get_obj_keys(*json);

            /* foreach json key*/
            LL_FOREACH(json_key, json_keys) {
                JSON_ENTITY *json_val = json_get(*json, json_key->data);
                if (json_val) {
                    if (json_val->type == JSON_STRING) {
                        sentence = json_to_string(json_val);
                        tokenizer_t *tok = tokenizer_nlp_sw(sentence, ml_get_stopwords(ml));
                        while ((token = tokenizer_next(tok)) != NULL) {
                            if (!strcmp(token, "\0")) continue;
                            set_put(json_bow_set, token);
                        }
                        tokenizer_free(tok);
                    }
                }
            }
            ml_init_vocabulary_from_json_bow_set(ml, json_bow_set);
        }
    }

    dict_free(json_bow_set, NULL);
}

bool check_local_maximum(int epoch, const float *med_losses) {
    int q = 0;
    if (epoch > 3) {
        q = epoch;
        while (q >= epoch - 4) {
            if (med_losses[q] < med_losses[q - 1]) {
                break;
            }
            q--;
        }
        if (epoch - q == 5) {
            return true;
        }
    }
    return false;
}

bool check_weigths(LogReg *model, float e) {
    float wt = 0;
    float wt1 = 0;
    float w = 0;
    for (int i = 1; i < model->weights_len; ++i) {
        wt = model->weights[i - 1];
        wt1 = model->weights[i];
        w = wt1 - wt;
        if (w < e) {
            return true;
        }
    }
    return false;
}

LogReg *train_model(int train_sz, Pair *train_set, float *bow_vector_1, float *bow_vector_2, STS *X, ML ml,
                    dictp json_dict, dictp vectors_dict, int mode, Pair *test_set, int test_sz) {
    /* initialize the model */
    LogReg *model = lr_new(ml_bow_sz(ml), learning_rate);
    LogReg *models[epochs];
    URand ur_mini_batch = NULL;

    memset(models, 0, sizeof(LogReg *) * epochs);

    /* create mini batch unique random */
    ur_create(&ur_mini_batch, 0, train_sz - 1);
    float med_losses[epochs], *y_pred = NULL;
    int y[batch_size];
    int *y_test = malloc(test_sz * sizeof(int));
    float *losses = malloc(test_sz * sizeof(float));
    float *result_vec = malloc(batch_size * ml_bow_sz(ml) * sizeof(float));
    float *result_vec_test = malloc(test_sz * ml_bow_sz(ml) * sizeof(float));
    int e = 0;
    for (e = 0; e < epochs; e++) {
        printf("epoch: %d\n", e);
        for (int j = 0; j < train_sz / batch_size; j++) {

            prepare_set(0, batch_size, true, ur_mini_batch, X, ml, json_dict, vectors_dict,
                        &train_set, result_vec, y, mode, 0);

            lr_train(model, result_vec, y, batch_size);
        }

        prepare_set(0, test_sz, false, NULL, X, ml, json_dict, vectors_dict,
                    &test_set, result_vec_test, y_test, mode, 0);

        y_pred = lr_predict(model, result_vec_test, test_sz);

        /* Calculate the max losses value & save into med_losses array*/
        med_losses[e] = calc_med_loss(losses, y_pred, y_test, test_sz);

        free(y_pred);

        /* Copy model & max losses*/
        lr_cpy(&models[e], model);

        /* Check if the last five max losses are ascending */
        if (check_local_maximum(e, med_losses)) {
            model = models[e - 4];
            break;
        }

        // /* Check if the last five max losses are ascending */
        // if (check_weigths(model,  -5)) {
        //     model = models[e - 4];
        //     break;
        // }

        ur_reset(ur_mini_batch);
    }

    for (int i = 0; i < epochs; ++i) {
        if (models[i] != NULL && models[i] != model) {
            lr_free(models[i]);
        }
    }

    /* Destroy unique random object */
    ur_destroy(&ur_mini_batch);
    free(result_vec);
    free(result_vec_test);
    free(losses);
    free(y_test);
    printf("\ntotal epochs: %d\n", e);
    return model;
}

dictp init_vectors_dict(dictp json_dict, ML ml, bool tfidf) {
    dictp vectors_dict = dict_new2(128, ml_bow_sz(ml) * sizeof(float));
    dict_config(vectors_dict,
                DICT_CONF_HASH_FUNC, djb2_str,
                DICT_CONF_KEY_CPY, strncpy,
                DICT_CONF_CMP, strncmp,
                DICT_CONF_KEY_SZ_F, str_sz,
                DICT_CONF_DONE
    );
    char *entry;
    ulong i_start = 0;
    int size = ml_bow_sz(ml);
    DICT_FOREACH_ENTRY(entry, json_dict, &i_start, json_dict->htab->buf_load) {
        float vector[size];
        int wc = 0;
        JSON_ENTITY **json = (JSON_ENTITY **) dict_get(json_dict, entry);
        ml_bow_json_vector(ml, *json, vector, &wc, false);
        if (tfidf) {
            ml_tfidf(ml, vector, wc);
        }
        dict_put(vectors_dict, entry, &vector);
    }
    return vectors_dict;
}

int main(int argc, char *argv[]) {
    clock_t begin = clock();
    float *y_pred = NULL, *result_vec_val = NULL, *bow_vector_1 = NULL, *bow_vector_2 = NULL;
    int *y_val = NULL, similar_sz = 0, different_sz = 0, train_sz = 0, test_sz = 0, val_sz = 0;
    int chunks = 0;
    Pair *train_set = NULL, *test_set = NULL, *val_set = NULL, *different_pairs = NULL, *similar_pairs = NULL;
    LogReg *model = NULL;
    dictp json_dict = NULL;
    setp train_json_files_set = NULL;
    ML ml = NULL;

    Options options = {NULL,
                       NULL,
                       NULL,
                       NULL,
                       NULL
    };

    /* parse arguments*/
    read_options(argc, argv, &options);

    /* initialize mode {tfidf|bow}*/
    bool tfidf = !strcmp(options.vec_mode, "tfidf");

    /* initialze job scheduler */
    if (THREADS) {
        js_create(&js, THREADS);
    }

    /* initialize an STS dataset X*/
    STS *X = init_sts_dataset_X(options.dataset_dir);

    /* iterate in dataset X in order to get the path for each json file*/
    init_json_dict(X, &json_dict, options.dataset_dir);

    /* read labelled_dataset_path file*/
    read_labelled_dataset_csv(X, options.labelled_dataset_path, "1");

    /* read labelled dataset csv*/
    read_labelled_dataset_csv(X, options.labelled_dataset_path, "0");

    /* init similar matches */
    chunks = 0;
    init_similar_pairs(stdout, X, &similar_pairs, &chunks, &similar_sz);

    /* init different matches*/
    chunks = 0;
    init_different_pairs(stdout, X, &different_pairs, &chunks, &different_sz);

    /* split and sort dataset into sorted matches array */
    split(similar_pairs, different_pairs, similar_sz, different_sz, &train_sz, &train_set, &test_sz, &test_set, &val_sz,
          &val_set);

    /******************************************* Prepare For Training *************************************************/

    /* Create ml object */
    ml_create(&ml, options.stop_words_path, json_dict->htab->buf_load);

    /* initialize json train set */
    init_json_train_set(&train_json_files_set, train_sz, train_set);

    /* tokenize json train set to init bag of words dict*/
    tokenize_json_train_set(ml, train_json_files_set, json_dict);

    if (tfidf) {
        ml_idf_remove(ml);  //TODO: <------- keep only 1000 with lowest idf value
    }
    /* Vectorize JSONs*/
    dictp vectors_dict = init_vectors_dict(json_dict, ml, tfidf);


    /* export vocabulary into csv file */
    ml_export_vocabulary(ml, options.export_path);

    result_vec_val = malloc(val_sz * ml_bow_sz(ml) * sizeof(float));

    /*********************************************** Training *********************************************************/

    model = train_model(train_sz, train_set, bow_vector_1, bow_vector_2, X, ml, json_dict, vectors_dict, tfidf,
                        test_set, test_sz);

    lr_export_model(model, !tfidf, options.export_path);

    /**************************************************** Predict ****************************************************/

    y_val = malloc(val_sz * sizeof(int));

    prepare_set(0, val_sz, false, NULL, NULL, ml, json_dict, vectors_dict, &val_set,
                result_vec_val, y_val, tfidf, 1);

    /* Predict validation set */
    y_pred = lr_predict(model, result_vec_val, val_sz);
    for (int i = 0; i < val_sz; i++) {
        if ((val_set[i].relation == 0 && y_pred[i] < 0.5) || (val_set[i].relation == 1 && y_pred[i] >= 0.5)) {
            printf(B_GREEN"spec1: %s, spec2: %s, y: %d, y_pred:%f\n"RESET, val_set[i].spec1, val_set[i].spec2,
                   val_set[i].relation, y_pred[i]);
        } else {
            printf(B_RED"spec1: %s, spec2: %s, y: %d, y_pred:%f\n"RESET, val_set[i].spec1, val_set[i].spec2,
                   val_set[i].relation, y_pred[i]);
        }
        // if (i==2) break;
    }

    /* calculate F1 score */
    printf("\nf1 score: %f\n\n", ml_f1_score((float *) y_val, y_pred, val_sz));

    if (THREADS) {
        js_destroy(&js);
    }

    free(result_vec_val);

    free(similar_pairs);
    free(different_pairs);

    free(train_set);
    free(test_set);
    free(val_set);
    ml_destroy(&ml);

    /* Destroy json dict */
    dict_free(json_dict, (void (*)(void *)) free_json_ht_ent);
    dict_free(vectors_dict, NULL);
    set_free(train_json_files_set);
    lr_free(model);
    free(y_pred);
    free(y_val);
    /* Destroy STS dataset X */
    sts_destroy(X);
    printf(WARNING"Time spend: %f\n"RESET, (double) (clock() - begin) / CLOCKS_PER_SEC);
    return 0;
}
