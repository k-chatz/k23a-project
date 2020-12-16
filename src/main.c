#include <stdio.h>
#include <fcntl.h>
#include <dirent.h>
#include <stdlib.h>
#include <string.h>

#include "../include/lists.h"
#include "../include/spec_to_specs.h"
#include "../include/spec_ids.h"
#include "../include/json_parser.h"
#include "../include/ml.h"

void readOptions(int argc, char **argv, char **dir, char **csv, char **sw) {
    int i;
    char *opt, *optVal;
    for (i = 1; i < argc; ++i) {
        opt = argv[i];
        optVal = argv[i + 1];
        if (strcmp(opt, "-dir") == 0) {
            if (optVal != NULL && optVal[0] != '-') {
                *dir = optVal;
            }
        } else if (strcmp(opt, "-csv") == 0) {
            if (optVal != NULL && optVal[0] != '-') {
                *csv = optVal;
            }
        } else if (strcmp(opt, "-sw") == 0) {
            if (optVal != NULL && optVal[0] != '-') {
                *sw = optVal;
            }
        }
    }
}

void free_json_ht_ent(JSON_ENTITY **val) {
    json_entity_free(*val);
}

void read_csv(STS *dataset_X, char *csv, char *flag) {

    FILE *fp = fopen(csv, "r");
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
    STS *X = NULL;
    ML ml = NULL;
    /* Initialize an STS dataset X*/
    X = init_sts_dataset_X(options.dataset_folder);
        JSON_ENTITY *ent = json_parse_file(json_path);
        htab_put(json_ht, key, &ent);
    }
    /* read_csv(dataset_X, csv, "0"); */
    read_csv(dataset_X, csv, "0");
    //printf("\n\n\n\n");
    //print_sts_diff(stdout, dataset_X);
    ml_create(&ml, sw, json_ht->buf_load);
    /* print_sts_diff(stdout, dataset_X); */
    ulong state = 0;
    while ((ptr = htab_iterate_r(json_ht, &state))) {
        JSON_ENTITY **json = (JSON_ENTITY **) (ptr + json_ht->key_sz);
        ml_tokenize_json(ml, *json);
    }
    ptr = NULL;
    float *vector = NULL;
    state = 0;
    int wc;
    while ((ptr = htab_iterate_r(json_ht, &state))) {
        JSON_ENTITY **json = (JSON_ENTITY **) (ptr + json_ht->key_sz);
        vector = ml_bow_json_vector(ml, *json, &wc);
        ml_tfidf(ml, vector, wc);
        // print_vector(ml, vector);
    }
    // print_bow_dict(ml);
    printf("bow_dict load: %ld\n", ml_get_bow_size(ml));
    printf("json_ht load: %ld\n", json_ht->buf_load);
    sts_destroy(dataset_X);
    htab_free_entries(json_ht, (void (*)(void *)) free_json_ht_ent);
    free(json_ht);
    return 0;
}
