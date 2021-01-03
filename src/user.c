#include <string.h>
#include <dirent.h>

#include "../include/spec_to_specs.h"
#include "../include/ml.h"
#include "../include/logreg.h"
#include "../include/unique_rand.h"

typedef struct options {
    char *user_json_files_path;
    char *csv_pairs;
    char *vocabulary;
    char *model;
} Options;

void read_options(int argc, char **argv, Options *o) {
    int i;
    char *opt, *optVal;
    for (i = 1; i < argc; ++i) {
        opt = argv[i];
        optVal = argv[i + 1];
        if (strcmp(opt, "-dir") == 0) {
            if (optVal != NULL && optVal[0] != '-') {
                o->user_json_files_path = optVal;
            }
        } else if (strcmp(opt, "-csv") == 0) {
            if (optVal != NULL && optVal[0] != '-') {
                o->csv_pairs = optVal;
            }
        } else if (strcmp(opt, "-vocabulary") == 0) {
            if (optVal != NULL && optVal[0] != '-') {
                o->vocabulary = optVal;
            }
        } else if (strcmp(opt, "-model") == 0) {
            if (optVal != NULL && optVal[0] != '-') {
                o->model = optVal;
            }
        }
    }
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

void
prepare_set(int p_start, int p_end, float *bow_vector_1, float *bow_vector_2, bool random, URand ur, STS *X, ML ml,
            dictp json_dict, Pair **pairs, float *result_vector, int *y, bool mode, bool is_user) {
    int wc = 0, x = 0;
    JSON_ENTITY **json1 = NULL, **json2 = NULL;
    SpecEntry *spec1 = NULL, *spec2 = NULL;
    for (int i = p_start; i < p_end; i++) {
        x = random ? ur_get(ur) : i;
        json1 = (JSON_ENTITY **) dict_get(json_dict, (*pairs)[x].spec1);
        ml_bow_json_vector(ml, *json1, bow_vector_1, &wc);
        if (mode) {
            ml_tfidf(ml, bow_vector_1, wc);
        }
        if (is_user == 0) {
            spec1 = sts_get(X, (*pairs)[x].spec1);
        }
        json2 = (JSON_ENTITY **) dict_get(json_dict, (*pairs)[x].spec2);
        ml_bow_json_vector(ml, *json2, bow_vector_2, &wc);
        if (mode) {
            ml_tfidf(ml, bow_vector_2, wc);
        }
        if (is_user == 0) {
            spec2 = sts_get(X, (*pairs)[x].spec2);
        }
        for (int c = 0; c < ml_bow_sz(ml); c++) {
            result_vector[(i - p_start) * ml_bow_sz(ml) + c] = fabs((bow_vector_1[c] - bow_vector_2[c]));
        }
        if (is_user == 0) {
            y[i - p_start] = (findRoot(X, spec1) == findRoot(X, spec2));
        }
    }
}

void free_json_ht_ent(JSON_ENTITY **val) {
    json_entity_free(*val);
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
        //fprintf(stdout, "Save to pairs array: %s, %s\n", left_spec_id, right_spec_id);
        (*pairs)[*counter].spec1 = strdup(left_spec_id);
        (*pairs)[*counter].spec2 = strdup(right_spec_id);
        (*pairs)[*counter].relation = -1;
        (*pairs)[*counter].type = NAT;
        (*counter)++;
    }
    fclose(fp);
}

int main(int argc, char *argv[]) {
    float *y_pred = NULL, *result_vec_val = NULL, *bow_vector_1 = NULL, *bow_vector_2 = NULL;
    float *result_vec_user = NULL;
    int *y_user = NULL, user_dataset_size = 0;
    char *entry = NULL;
    FILE *fp = NULL;
    Pair *user_pairs = NULL;
    LogReg *model = NULL;
    ML ml = NULL;
    Options options = {NULL, NULL,};

    /* parse arguments*/
    read_options(argc, argv, &options);

    ml_create(&ml, NULL, 0);

    /* init vocabulary*/
    fp = fopen(options.vocabulary, "r");
    ml_init_vocabulary(ml, fp);
    fclose(fp);

    /* init model*/
    fp = fopen(options.model, "r");
    model = lr_new_from_file(fp);
    fclose(fp);

    y_user = malloc(user_dataset_size * sizeof(float));

    bow_vector_1 = malloc(ml_bow_sz(ml) * sizeof(float));

    bow_vector_2 = malloc(ml_bow_sz(ml) * sizeof(float));

    /* Read user dataset */
    read_user_labelled_dataset_csv(options.csv_pairs, &user_pairs, &user_dataset_size);

    result_vec_user = malloc(user_dataset_size * ml_bow_sz(ml) * sizeof(float));
    dictp user_dataset_dict = user_json_dict(options.user_json_files_path);

    ulong i_state = 0;
    HSET_FOREACH_ENTRY(entry, user_dataset_dict, &i_state, user_dataset_dict->htab->buf_load) {
        printf("[%s]\n", entry);
    }

    prepare_set(0, user_dataset_size, bow_vector_1, bow_vector_2, false, NULL, NULL, ml,
                user_dataset_dict, &user_pairs, result_vec_user, y_user, false, 1);

    /* Predict user dataset */
    y_pred = lr_predict(model, result_vec_user, user_dataset_size);

    for (int i = 0; i < user_dataset_size; i++) {
        printf("spec1: %s, spec2: %s, y_pred:%f\n", user_pairs[i].spec1, user_pairs[i].spec2, y_pred[i]);
    }

    dict_free(user_dataset_dict, (void (*)(void *)) free_json_ht_ent);

    ml_destroy(&ml);

    return 0;
}
