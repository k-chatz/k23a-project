#include <stdio.h>
#include <fcntl.h>
#include <dirent.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <math.h>

#include "../include/lists.h"
#include "../include/spec_to_specs.h"
#include "../include/json_parser.h"
#include "../include/ml.h"
#include "../include/logreg.h"
#include "../include/unique_rand.h"

#define epochs 40
#define batch_size 100

typedef struct options {
    char *dataset_folder;
    char *labelled_dataset_path;
    char *stop_words_path;
} Options;

void read_options(int argc, char **argv, Options *o) {
    int i;
    char *opt, *optVal;
    for (i = 1; i < argc; ++i) {
        opt = argv[i];
        optVal = argv[i + 1];
        if (strcmp(opt, "-dir") == 0) {
            if (optVal != NULL && optVal[0] != '-') {
                o->dataset_folder = optVal;
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

    /* scan external dataset_folder*/
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
    char json_website[128], json_num[128], json_path[280], *entry = NULL, **json_train_keys = NULL;
    int wc = 0, rand_pos1 = 0, rand_pos2 = 0;
    Options options = {NULL, NULL, NULL};
    UniqueRand ur = NULL;
    ulong iterate_state = 0;
    STS *X = NULL;
    hashp json_ht = NULL;
    ML ml = NULL;
    JSON_ENTITY **json = NULL;

    /* Parse arguments*/
    read_options(argc, argv, &options);

    /* Initialize an STS dataset X*/
    X = init_sts_dataset_X(options.dataset_folder);

    /* Read labelled_dataset_path file*/
    read_labelled_dataset_csv(X, options.labelled_dataset_path, "1");

    /* print result*/
    //print_sts(stdout, X);
    //print_sts_similar(stdout, X);

    /* Create json hashtable*/
    json_ht = htab_new(djb2_str, 128, sizeof(JSON_ENTITY *), X->ht->htab->buf_cap);

    /* Iterate in dataset X in order to get the path for each  json file*/
    iterate_state = 0;
    // for ( int i = 0, entry = dict_iterate_r(X->ht, &iterate_state) ; entry!=NULL && i < 5 ; i++, entry = dict_iterate_r(X->ht, &iterate_state)) {
    // for (  int i = 0; (entry = dict_iterate_r(X->ht, &iterate_state)) && i < X->ht->htab->buf_load ; i++) {
    // for(int i = 0 ; i< 10; i++) {
    // while((entry = dict_iterate_r(X->ht, &iterate_state))){
    DICT_FOREACH_ENTRY(entry, X->ht, &iterate_state, X->ht->htab->buf_load) {

        /* Constructing the key for this JSON_ENTITY entry*/
        sscanf(entry, "%[^/]//%[^/]", json_website, json_num);

        /* Constructing the path*/
        snprintf(json_path, 280, "%s/%s/%s.json", options.dataset_folder, json_website, json_num);

        /* Opening, parsing and creating a JSON ENTITY object for the 'json_path' file*/
        JSON_ENTITY *ent = json_parse_file(json_path);

        /* Putting this JSON_ENTITY in 'json_ht' hashtable*/
        htab_put(json_ht, entry, &ent);
    }

    /* Read labelled dataset csv*/
    read_labelled_dataset_csv(X, options.labelled_dataset_path, "0");

    /* Print different STS*/
    //print_sts_diff(stdout, X);

    ml_create(&ml, options.stop_words_path, json_ht->buf_load);
    /* print_sst_diff(stdout, X); */

    /* Iterate in json hashtable and get the JSON_ENTITY for each json to tokenize it*/
    iterate_state = 0;


    int train_set_size =
            (json_ht->buf_load / 2) % 2 ? (int) (json_ht->buf_load / 2 - 1) : (int) (json_ht->buf_load / 2);

    json_train_keys = malloc((size_t) (train_set_size * sizeof(char *)));

    HT_FOREACH_ENTRY(entry, json_ht, &iterate_state, train_set_size) {
        json_train_keys[i] = entry;
        json = (JSON_ENTITY **) (entry + json_ht->key_sz);
        ml_tokenize_json(ml, *json);
    }

    ml_idf_remove(ml);

    JSON_ENTITY **json1 = NULL, **json2 = NULL;


    float *result_vec = malloc(batch_size * ml_get_bow_size(ml) * sizeof(float));

    LogReg *clf = logreg_new(ml_get_bow_size(ml), 0.001);
    int *y = malloc(batch_size * sizeof(int));
    SpecEntry *spec1, *spec2;

    ulong test_set_size = train_set_size + (ml_get_bow_size(ml) - train_set_size) / 2;

    char **json_test_keys = malloc((size_t) (test_set_size * sizeof(char *)));

    HT_FOREACH_ENTRY(entry, json_ht, (ulong *) &train_set_size, test_set_size) {
        json_test_keys[i] = entry;
    }
    ulong capacity = ml_get_bow_size(ml);
    float *bow_vector1 = malloc(capacity * sizeof(float));
    float *bow_vector2 = malloc(capacity * sizeof(float));

    /* Create a unique random object for ranges between 0 and train_set_size */
    ur_create(&ur, 0, train_set_size - 1);


    ///////////////////////////////////////////// Just a test for unique random values
    int num = 0;
    int i = 0;
    while (num >= 0) {
        num = ur_get(ur);
        printf("i:%d:::: %d\n", i, num);
        i++;
    }
    //////////////////////////////////////////////


    for (int e = 0; e < epochs; e++) {

        for (int j = 0; j < train_set_size / batch_size; j++) {
            for (int i = 0; i < batch_size; i++) {

                /* vector 1 */
                rand_pos1 = ur_get(ur);
                if (rand_pos1 >= 0) {
                    json1 = (JSON_ENTITY **) htab_get(json_ht, json_train_keys[rand_pos1]);
                    ml_bow_json_vector(ml, *json1, bow_vector1, &wc);
                    ml_tfidf(ml, bow_vector1, wc);
                    spec1 = sts_get(X, json_train_keys[rand_pos1]);
                } else {
                    fprintf(stderr, "ERROR!\n");
                }

                /* vector 2 */
                rand_pos2 = ur_get(ur);
                if (rand_pos2 >= 0) {
                    json2 = (JSON_ENTITY **) htab_get(json_ht, json_train_keys[rand_pos2]);
                    ml_bow_json_vector(ml, *json2, bow_vector2, &wc);
                    ml_tfidf(ml, bow_vector2, wc);
                    spec2 = sts_get(X, json_train_keys[rand_pos2]);
                } else {
                    fprintf(stderr, "ERROR!\n");
                }

                for (int c = 0; c < ml_get_bow_size(ml); c++) {
                    result_vec[i * ml_get_bow_size(ml) + c] = abs((int) (bow_vector1[i] - bow_vector2[i]));
                }

                y[i] = (findRoot(X, spec1) == findRoot(X, spec2));
            }
            train(clf, result_vec, y, batch_size);
        }


        ur_print(ur);
        ur_reset(ur);
        ur_print(ur);
    }

    free(bow_vector1);
    free(bow_vector2);

    // /* Iterate in json hashtable and get the JSON_ENTITY for each json to tokenize it*/
    // iterate_state = 0;
    // HT_FOREACH_ENTRY(entry, json_ht, &iterate_state, train_set_size) {
    //     json = (JSON_ENTITY **) (entry + json_ht->key_sz);
    //     vector = ml_bow_json_vector(ml, *json, &wc);
    //     ml_tfidf(ml, vector, wc);
    //     // print_vector(ml, vector);
    // }

    printf("json_ht load: %ld\n", json_ht->buf_load);

    ur_destroy(&ur);

    /* Destroy STS dataset X*/
    sts_destroy(X);

    /* Destroy */
    //htab_free_entries(json_ht, (void (*)(void *)) json_entity_free);
    htab_free_entries(json_ht, (void (*)(void *)) free_json_ht_ent);
    free(json_ht);

    return 0;
}

