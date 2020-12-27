#include <stdio.h>
#include <fcntl.h>
#include <dirent.h>
#include <stdlib.h>
#include <string.h>

#include "../include/lists.h"
#include "../include/spec_to_specs.h"
#include "../include/json_parser.h"
#include "../include/ml.h"
#include "../include/logreg.h"
#include "../include/unique_rand.h"
#include "../include/hset.h"

#define epochs 50
#define batch_size 34
#define learning_rate 0.0001

typedef struct options {
    char *dataset_dir;
    char *labelled_dataset_path;
    char *stop_words_path;
    char *user_dataset_file;
    char *vec_mode;
} Options;

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
        } else if (strcmp(opt, "-ds") == 0) {
            if (optVal != NULL && optVal[0] != '-') {
                o->user_dataset_file = optVal;
            }
        } else if (strcmp(opt, "-m") == 0){
            if (optVal != NULL && optVal[0] != '-') {
                o->vec_mode = optVal;
            }
        }
    }
}

void free_json_ht_ent(JSON_ENTITY **val) {
    json_entity_free(*val);
}

void read_labelled_dataset_csv(STS *dataset_X, char *sigmod_labelled_dataset, char *flag) {
    char left_spec_id[50], right_spec_id[50], label[50];
    FILE *fp = fopen(sigmod_labelled_dataset, "r");
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

void read_user_dataset_csv(char *user_dataset_file, Match *matches, int *counter) {
    assert(user_dataset_file != NULL);
    assert(*matches == NULL);
    assert(*counter == 0);
    FILE *fp = fopen(user_dataset_file, "r");
    char left_spec_id[50], right_spec_id[50];
    //skip first row
    fseek(fp, 27, SEEK_SET);
    while (fscanf(fp, "%[^,],%s\n", left_spec_id, right_spec_id) != EOF) {
        (*matches) = realloc(*matches, (*counter + 1) * sizeof(struct match));
        //fprintf(stdout, "Save to matches array: %s, %s\n", left_spec_id, right_spec_id);
        (*matches)[*counter].spec1 = strdup(left_spec_id);
        (*matches)[*counter].spec2 = strdup(right_spec_id);
        (*matches)[*counter].relation = -1;
        (*matches)[*counter].type = NAT;
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

void
prepare_set(int p_start, int p_end, float *bow_vector_1, float *bow_vector_2, bool random, UniqueRand ur, STS *X, ML ml,
            dictp json_dict, Match *matches, float *result_vector, int *y, bool mode) {
    int wc = 0, x = 0;
    JSON_ENTITY **json1 = NULL, **json2 = NULL;
    SpecEntry *spec1 = NULL, *spec2 = NULL;
    for (int i = p_start; i < p_end; i++) {
        x = random ? ur_get(ur) : i;
        json1 = (JSON_ENTITY **) dict_get(json_dict, (*matches)[x].spec1);
        ml_bow_json_vector(ml, *json1, bow_vector_1, &wc);
        if(mode){
            ml_tfidf(ml, bow_vector_1, wc);
        }
        spec1 = sts_get(X, (*matches)[x].spec1);

        json2 = (JSON_ENTITY **) dict_get(json_dict, (*matches)[x].spec2);
        ml_bow_json_vector(ml, *json2, bow_vector_2, &wc);
        if(mode){
            ml_tfidf(ml, bow_vector_2, wc);
        }
        spec2 = sts_get(X, (*matches)[x].spec2);
        for (int c = 0; c < ml_get_bow_size(ml); c++) {
            result_vector[(i - p_start) * ml_get_bow_size(ml) + c] = fabs((bow_vector_1[c] - bow_vector_2[c]));
        }
        y[i] = (findRoot(X, spec1) == findRoot(X, spec2));
    }
}

Match shuffle_dataset(Match matches, int dataset_size){
    UniqueRand ur = NULL;
    Match shuffle_matches = malloc(dataset_size * sizeof(struct match));
    int x = 0;
    ur_create(&ur, 0, dataset_size - 1);
    for (int i = 0; i < dataset_size; i++){
        x = ur_get(ur);
        shuffle_matches[i] = matches[x];
    }
    return shuffle_matches;
}


Match split_dataset(Match matches, setp json_train_keys, int train_set_size, int test_set_size, int dataset_size,
                    UniqueRand ur) {

    

    int x = 0;
    Match sorted_matches = malloc(dataset_size * sizeof(struct match));
    /* Split the dataset to TRAIN, TEST & VALIDATION sets */
    for (int i = 0; i < train_set_size; i++) {
        x = ur_get(ur);
        /* Collect randomly half of the dataset for the training */
        matches[x].type = TRAIN;
        if (!set_in(json_train_keys, matches[x].spec1)) {
            set_put(json_train_keys, matches[x].spec1);
        }
        if (!set_in(json_train_keys, matches[x].spec2)) {
            set_put(json_train_keys, matches[x].spec2);
        }
        memcpy(&sorted_matches[i], &matches[x], sizeof(struct match));
    }

    /* Collect randomly the test set */
    for (int i = train_set_size; i < test_set_size; i++) {
        x = ur_get(ur);
        matches[x].type = TEST;
        memcpy(&sorted_matches[i], &matches[x], sizeof(struct match));
    }

    /* Collect whats left to be the validation set */
    for (int i = test_set_size; i < dataset_size; i++) {
        x = ur_get(ur);
        matches[x].type = VALIDATION;
        memcpy(&sorted_matches[i], &matches[x], sizeof(struct match));
    }
    return sorted_matches;
}




float calc_max_loss(float *losses, float *y_pred, int *y, int offset) {
    float max_loss = 0;
    /* Calculate max_loss */
    for (int i = 0; i < offset; i++) {
        losses[i] = logloss(y_pred[i], y[i]);
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

int main(int argc, char *argv[]) {
    char json_website[128], json_num[128], json_path[280], *entry = NULL;
    int dataset_size = 0, chunks = 0, train_set_size = 0, test_set_size = 0;
    int user_dataset_size = 0, q = 0;
    float max_losses[epochs], *losses = NULL, *y_pred = NULL, *result_vec = NULL, *result_vec_test = NULL;
    float *bow_vector_1 = NULL, *bow_vector_2 = NULL;
    int *y = NULL;
    bool mode;
    LogReg *clf = NULL;
    LogReg models[epochs];
    Options options = {NULL, NULL, NULL, NULL, NULL};
    UniqueRand ur_dataset = NULL, ur_mini_batch = NULL;
    ulong i_state = 0;
    setp json_train_keys = NULL;
    STS *X = NULL;
    dictp json_dict = NULL;
    ML ml = NULL;
    JSON_ENTITY **json = NULL;
    Match matches = NULL, user_matches = NULL, sorted_matches = NULL;

    /* Parse arguments*/
    read_options(argc, argv, &options);

    mode = !strcmp(options.vec_mode, "tfidf");

    /* Initialize an STS dataset X*/
    X = init_sts_dataset_X(options.dataset_dir);

    /* Create json hashtable*/
    json_dict = dict_new3(128, sizeof(JSON_ENTITY *), X->dict->htab->buf_cap);
    dict_config(json_dict,
                DICT_CONF_HASH_FUNC, djb2_str,
                DICT_CONF_KEY_CPY, strncpy,
                DICT_CONF_CMP, strncmp,
                DICT_CONF_KEY_SZ_F, str_sz,
                DICT_CONF_DONE
    );

    /* Iterate in dataset X in order to get the path for each  json file*/
    i_state = 0;

    /* Create json hash table using dataset X */
    DICT_FOREACH_ENTRY(entry, X->dict, &i_state, X->dict->htab->buf_load) {

        /* Constructing the key for this JSON_ENTITY entry*/
        sscanf(entry, "%[^/]//%[^/]", json_website, json_num);

        /* Constructing the path*/
        snprintf(json_path, 280, "%s/%s/%s.json", options.dataset_dir, json_website, json_num);

        /* Opening, parsing and creating a JSON ENTITY object for the 'json_path' file*/
        JSON_ENTITY *ent = json_parse_file(json_path);

        /* Putting this JSON_ENTITY in 'json_dict' hashtable*/
        dict_put(json_dict, entry, &ent);
    }

    /* Read labelled_dataset_path file*/
    read_labelled_dataset_csv(X, options.labelled_dataset_path, "1");

    /* Read labelled dataset csv*/
    read_labelled_dataset_csv(X, options.labelled_dataset_path, "0");

    /* init similar*/
    sts_similar(stdout, X, &matches, &chunks, &dataset_size);
    //print_sts_similar(stdout, X);

    /* Print different STS*/
    //print_sts_diff(stdout, X);

    /* init different*/
    sts_different(stdout, X, &matches, &chunks, &dataset_size);

    /**** Training ****/

    /* Create ml object */
    ml_create(&ml, options.stop_words_path, json_dict->htab->buf_load);

    /* Iterate in json hashtable and get the JSON_ENTITY for each json to tokenize it */
    i_state = 0;

    train_set_size = (dataset_size / 2) % 2 ? (int) (dataset_size / 2 - 1) : (int) (dataset_size / 2);

    test_set_size = train_set_size + (dataset_size - train_set_size) / 2;

    ur_create(&ur_dataset, 0, dataset_size - 1);

    json_train_keys = set_new(128);
    dict_config(json_train_keys,
                DICT_CONF_CMP, (ht_cmp_func) strncmp,
                DICT_CONF_KEY_CPY, (ht_key_cpy_func) strncpy,
                DICT_CONF_HASH_FUNC, djb2_str,
                DICT_CONF_KEY_SZ_F, str_sz,
                DICT_CONF_DONE
    );


    //TODO: shuffle the dataset
    Match shuffled_dataset = shuffle_dataset(matches, dataset_size);


    /* Split and sort dataset into sorted matches array */
    sorted_matches = split_dataset(shuffled_dataset, json_train_keys, train_set_size, test_set_size, dataset_size, ur_dataset);

    i_state = 0;
    for (keyp k = set_iterate_r(json_train_keys, &i_state); k != NULL; k = set_iterate_r(json_train_keys, &i_state)) {
        // printf("%s\n", (char*)k);
        json = (JSON_ENTITY **) dict_get(json_dict, k);
        ml_tokenize_json(ml, *json);
    }
    if (mode)
        ml_idf_remove(ml);

    result_vec = malloc(batch_size * ml_get_bow_size(ml) * sizeof(float));
    result_vec_test = malloc((test_set_size - train_set_size) * ml_get_bow_size(ml) * sizeof(float));
    clf = logreg_new(ml_get_bow_size(ml), learning_rate);

    y = malloc(batch_size * sizeof(int));

    losses = malloc((test_set_size - train_set_size - 1) * sizeof(float));

    /* Create mini batch unique random */
    ur_create(&ur_mini_batch, 0, train_set_size - 1);

    bow_vector_1 = malloc(ml_get_bow_size(ml) * sizeof(float));

    bow_vector_2 = malloc(ml_get_bow_size(ml) * sizeof(float));
     
    /**** Epochs loop ****/
    for (int e = 0; e < epochs; e++) {

        for (int j = 0; j < train_set_size / batch_size; j++) {
            prepare_set(0, batch_size, bow_vector_1, bow_vector_2, true, ur_mini_batch, X, ml, json_dict,
                        &sorted_matches, result_vec, y, mode);
            train(clf, result_vec, y, batch_size);
        }
        prepare_set(train_set_size, test_set_size, bow_vector_1, bow_vector_2, false, NULL, X, ml, json_dict,
                    &sorted_matches, result_vec_test, y, mode);

        y_pred = predict(clf, result_vec_test, (test_set_size - train_set_size));

        /* Calculate the max losses value & save into max_losses array*/
        max_losses[e] = calc_max_loss(losses, y_pred, y, test_set_size - train_set_size);

        /* Copy model & max losses*/
        memcpy(&models[e], clf, sizeof(LogReg));

        /* Check if the last five max losses are ascending */
        if (e > 3) {
            q = e;
            while (q >= e - 4) {
                if (max_losses[q] < max_losses[q - 1]) {
                    break;
                }
                q--;
            }
            if (e - q == 5) {
                clf = &models[e - 4];
                break;
            }
        }

        ur_reset(ur_mini_batch);



        float *old_weight = clf->weights;
    }

    /**** Predict ****/

    //TODO: calculate F1 score

    prepare_set(test_set_size, dataset_size, bow_vector_1, bow_vector_2, false, NULL, X, ml,
                json_dict, &sorted_matches, result_vec_test, y, mode);

    /* Predict validation set */
    y_pred = predict(clf, result_vec_test, (dataset_size - test_set_size));
    for (int i = test_set_size; i < dataset_size; i++) {
        printf("spec1: %s, spec2: %s, y: %d, y_pred: %f\n",sorted_matches[i].spec1, sorted_matches[i].spec2, sorted_matches[i].relation, y_pred[i-test_set_size]);
    }

    //TODO: calculate user dataset score

    /* Read user dataset */
    // read_user_dataset_csv(options.user_dataset_file, &user_matches, &user_dataset_size);




    // prepare_set(0, user_dataset_size, bow_vector_1, bow_vector_2, false, NULL, X, ml,
    //             json_dict, &user_matches, result_vec_test, y);

    /* Predict user dataset */
    // y_pred = predict(clf, result_vec_test, (test_set_size - train_set_size));

    free(losses);
    free(bow_vector_1);
    free(bow_vector_2);

    /* Destroy unique random objects */
    ur_destroy(&ur_mini_batch);
    ur_destroy(&ur_dataset);

    /* Destroy STS dataset X */
    sts_destroy(X);

    /* Destroy json dict */
    dict_free(json_dict, (void (*)(void *)) free_json_ht_ent);

    return 0;
}
