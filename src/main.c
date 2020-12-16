#include <stdio.h>
#include <fcntl.h>
#include <dirent.h>
#include <stdlib.h>
#include <string.h>

#include "../include/lists.h"
#include "../include/spec_to_specs.h"
#include "../include/json_parser.h"
#include "../include/ml.h"

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
    char json_website[128], json_num[128], json_path[280], *entry = NULL;
    float *vector = NULL;
    int wc = 0;
    Options options = {NULL, NULL, NULL};
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
    DICT_FOREACH_ENTRY(entry, X->ht, &iterate_state) {
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
    HT_FOREACH_ENTRY(entry, json_ht, &iterate_state) {
        json = (JSON_ENTITY **) (entry + json_ht->key_sz);
        ml_tokenize_json(ml, *json);
    }

    /* Iterate in json hashtable and get the JSON_ENTITY for each json to tokenize it*/
    iterate_state = 0;
    HT_FOREACH_ENTRY(entry, json_ht, &iterate_state) {
        vector = ml_bow_json_vector(ml, *json, &wc);
        ml_tfidf(ml, vector, wc);
        // print_vector(ml, vector);
    }

    // print_bow_dict(ml);
    printf("bow_dict load: %ld\n", ml_get_bow_size(ml));
    printf("json_ht load: %ld\n", json_ht->buf_load);

    /* Destroy STS dataset X*/
    sts_destroy(X);

    /* Destroy */
    //htab_free_entries(json_ht, (void (*)(void *)) json_entity_free);
    htab_free_entries(json_ht, (void (*)(void *)) free_json_ht_ent);
    free(json_ht);

    return 0;
}
