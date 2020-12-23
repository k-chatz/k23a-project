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

#define epochs 40
#define batch_size 100

typedef struct options {
    char *dataset_dir;
    char *labelled_dataset_path;
    char *stop_words_path;
    char *user_dataset_path;
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
        }
        else if (strcmp(opt, "-ds") == 0) {
            if (optVal != NULL && optVal[0] != '-') {
                o->user_dataset_path = optVal;
            }
        }
    }
}

void free_json_ht_ent(JSON_ENTITY **val) {
    json_entity_free(*val);
}

void read_labelled_dataset_csv(STS *dataset_X, char *sigmod_labelled_dataset, char *flag) {
    FILE *fp = fopen(sigmod_labelled_dataset, "r");
    char left_spec_id[50], right_spec_id[50], label[50];
    //skip first row fseek
    uint merges = 0, diffs = 0;
    fseek(fp, 33, SEEK_SET);
    while (fscanf(fp, "%[^,],%[^,],%s\n", left_spec_id, right_spec_id, label) != EOF) {
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

int main(int argc, char *argv[]) {
    char json_website[128], json_num[128], json_path[280], *entry = NULL;
    int wc = 0, rand_pos1 = 0, rand_pos2 = 0, dataset_size = 0, chunks = 0, train_set_size = 0, test_set_size = 0, x = 0;
    Options options = {NULL, NULL, NULL, NULL};
    UniqueRand ur = NULL;
    UniqueRand ur_dataset = NULL;
    ulong i_state = 0;
    setp json_train_keys = NULL;
    STS *X = NULL;
    dictp json_dict = NULL;
    ML ml = NULL;
    JSON_ENTITY **json = NULL;
    Match matches = NULL;
    Match *sorted_matches = NULL;

    /* Parse arguments*/
    read_options(argc, argv, &options);

    /* Initialize an STS dataset X*/
    X = init_sts_dataset_X(options.dataset_dir);

    /* Create json hashtable*/
    json_dict = dict_new3(128, sizeof(JSON_ENTITY *), X->ht->htab->buf_cap);
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
    DICT_FOREACH_ENTRY(entry, X->ht, &i_state, X->ht->htab->buf_load) {

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

    /* print result*/
    print_sts(stdout, X, &matches, &chunks, &dataset_size);
    //print_sts_similar(stdout, X);

    /* Print different STS*/
    //print_sts_diff(stdout, X);

    print_sts_differences(stdout, X, &matches, &chunks, &dataset_size);

    /**** Training ****/

    /* Create ml object */
    ml_create(&ml, options.stop_words_path, json_dict->htab->buf_load);

    /* Iterate in json hashtable and get the JSON_ENTITY for each json to tokenize it */
    i_state = 0;

    train_set_size = (dataset_size / 2) % 2 ? (int) (dataset_size / 2 - 1) : (int) (dataset_size / 2);

    test_set_size = train_set_size + (dataset_size - train_set_size) / 2;

    ur_create(&ur_dataset, 0, dataset_size - 1);

    /*  */
    json_train_keys = set_new(128);
    dict_config(json_train_keys,
                DICT_CONF_CMP, (ht_cmp_func) strncmp,
                DICT_CONF_KEY_CPY, (ht_key_cpy_func) strncpy,
                DICT_CONF_HASH_FUNC, djb2_str,
                DICT_CONF_KEY_SZ_F, str_sz,
                DICT_CONF_DONE
    );

    sorted_matches = malloc(dataset_size * sizeof(Match));

    /* Split the dataset to TRAIN, TEST & VALIDATION sets */
    for (int i = 0; i < train_set_size; i++) {
        x = ur_get(ur_dataset);

        /* Collect randomly half of the dataset for the training */
        matches[x].type = TRAIN;
        if (!set_in(json_train_keys, matches[x].spec1)) {
            set_put(json_train_keys, matches[x].spec1);
        }

        if (!set_in(json_train_keys, matches[x].spec2)) {
            set_put(json_train_keys, matches[x].spec2);
        }

        sorted_matches[i] = &matches[x];
    }

    /* Collect randomly the test set */
    for (int i = train_set_size; i < test_set_size; i++) {
        x = ur_get(ur_dataset);
        matches[x].type = TEST;
        sorted_matches[i] = &matches[x];
    }

    /* Collect whats left to be the validation set */
    for (int i = test_set_size; i < dataset_size; i++) {
        x = ur_get(ur_dataset);
        matches[x].type = VALIDATION;
        sorted_matches[i] = &matches[x];
    }

    for (int i = train_set_size; i < test_set_size; i++) {
        if (sorted_matches[i]->type == TRAIN)
            printf("i: %d, type: TRAIN\n", i);
        else if (sorted_matches[i]->type == TEST) {
            printf("i: %d, type: TEST\n", i);
        } else if (sorted_matches[i]->type == VALIDATION) {
            printf("i: %d, type: VALIDATION\n", i);
        } else {
            printf("i: %d, NAT!\n", i);
        }
    }

    i_state = 0;
    for (keyp k = set_iterate_r(json_train_keys, &i_state); k != NULL; k = set_iterate_r(json_train_keys, &i_state)) {
        // printf("%s\n", (char*)k);
        json = (JSON_ENTITY **) dict_get(json_dict, k);
        ml_tokenize_json(ml, *json);
    }

    ml_idf_remove(ml);

    putchar('\n');

    float *result_vec = malloc(batch_size * ml_get_bow_size(ml) * sizeof(float));
    float *result_vec_test = malloc((test_set_size - train_set_size) * ml_get_bow_size(ml) * sizeof(float));

    LogReg *clf = logreg_new(ml_get_bow_size(ml), (float) 0.001);

    int *y = malloc(batch_size * sizeof(int));

    UniqueRand ur_mini_batch = NULL;
    ur_create(&ur_mini_batch, 0, train_set_size - 1);

    UniqueRand ur_test = NULL;
    ur_create(&ur_test, train_set_size, test_set_size - 1);

    JSON_ENTITY **json1 = NULL, **json2 = NULL;
    float *bow_vector1 = malloc(ml_get_bow_size(ml) * sizeof(float));
    float *bow_vector2 = malloc(ml_get_bow_size(ml) * sizeof(float));
    SpecEntry *spec1, *spec2;
    float *y_pred = NULL;

    LogReg *clf_cp = malloc(sizeof(LogReg));
    float *loss = malloc((test_set_size - train_set_size - 1) * sizeof(float));

    /* Epochs loop*/
    for (int e = 0; e < epochs; e++) {
        for (int j = 0; j < train_set_size / batch_size; j++) {
            for (int i = 0; i < batch_size; i++) {
                x = ur_get(ur_mini_batch);
                json1 = (JSON_ENTITY **) dict_get(json_dict, sorted_matches[x]->spec1);
                ml_bow_json_vector(ml, *json1, bow_vector1, &wc);
                ml_tfidf(ml, bow_vector1, wc);
                spec1 = sts_get(X, sorted_matches[x]->spec1);

                json2 = (JSON_ENTITY **) dict_get(json_dict, sorted_matches[x]->spec2);
                ml_bow_json_vector(ml, *json2, bow_vector2, &wc);
                ml_tfidf(ml, bow_vector2, wc);
                spec2 = sts_get(X, sorted_matches[x]->spec2);
                for (int c = 0; c < ml_get_bow_size(ml); c++) {
                    result_vec[i * ml_get_bow_size(ml) + c] = abs((int) (bow_vector1[i] - bow_vector2[i]));
                }
                y[i] = (findRoot(X, spec1) == findRoot(X, spec2));
            }
            train(clf, result_vec, y, batch_size);
        }

        // TODO: predict test set
        // Mazevoume ta apotelesmata se ena pinaka
        // trexw to logloss y_test kai y_pred
        // krataw meso h megisto 
        // krataw se kathe epoch ena antigrafo tou montelou 
        // an dw oti tis epomenes 5 fores anevainei to loss
        // kanw break kai gynaw sto antigrafo.

        float max_loss = -1;

        for (int i = train_set_size; i < test_set_size; i++) {
            json1 = (JSON_ENTITY **) dict_get(json_dict, sorted_matches[i]->spec1);
            ml_bow_json_vector(ml, *json1, bow_vector1, &wc);
            ml_tfidf(ml, bow_vector1, wc);
            spec1 = sts_get(X, sorted_matches[i]->spec1);

            json2 = (JSON_ENTITY **) dict_get(json_dict, sorted_matches[i]->spec2);
            ml_bow_json_vector(ml, *json2, bow_vector2, &wc);
            ml_tfidf(ml, bow_vector2, wc);
            spec2 = sts_get(X, sorted_matches[i]->spec2);

            for (int c = 0; c < ml_get_bow_size(ml); c++) {
                result_vec_test[(i - train_set_size) * ml_get_bow_size(ml) + c] = abs(
                        (int) (bow_vector1[i] - bow_vector2[i]));
            }
            y[i] = (findRoot(X, spec1) == findRoot(X, spec2));
        }

        y_pred = predict(clf, result_vec_test, (test_set_size - train_set_size - 1));
        memcpy(clf_cp, clf, sizeof(LogReg));
        for (int i = 0; i < test_set_size - train_set_size - 1; i++) {
            loss[i] = logloss(y_pred[i], y[i]);
            if (i == 0) {
                max_loss = loss[i];
            } else {
                if (max_loss < loss[i]) {
                    max_loss = loss[i];
                }
            }
        }


        ur_reset(ur_mini_batch);
    }

    putchar('\n');

    //TODO: predict validation set, ypologismos score (px F1)
    //TODO: Predict to montelo tou user, ypologismos score

    // Predict

    // read user dataset file
//////////////////////////////////////////////////////////////////////////////////////////////////
    free(loss);
    free(clf_cp);

    free(bow_vector1);
    free(bow_vector2);

    ur_destroy(&ur_mini_batch);
    ur_destroy(&ur_dataset);

    /* Destroy STS dataset X*/
    sts_destroy(X);

    /* Destroy json dict*/
    dict_free(json_dict, (void (*)(void *)) free_json_ht_ent);

    return 0;
}
