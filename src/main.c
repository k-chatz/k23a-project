#include <stdio.h>
#include <fcntl.h>

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

int main(int argc, char *argv[]) {
    char *dir = NULL, *csv = NULL, *sw = NULL, json_website[128], json_num[128], json_path[280], *ptr = NULL;
    ML ml = NULL;
    STS *dataset_X = NULL;
    readOptions(argc, argv, &dir, &csv, &sw);
    dataset_X = get_spec_ids(dir);
    read_csv(dataset_X, csv, "1");
    //print result
    //print_sts(stdout, dataset_X);
    //print_sts_similar(stdout, dataset_X);
    hashp json_ht = htab_new(djb2_str, 128, sizeof(JSON_ENTITY *), dataset_X->ht->htab->buf_cap);
    ulong iter_state = 0;
    for (char *key = dict_iterate_r(dataset_X->ht, &iter_state);
         key != NULL; key = dict_iterate_r(dataset_X->ht, &iter_state)) {
        sscanf(key, "%[^/]//%[^/]", json_website, json_num);
        snprintf(json_path, 280, "%s/%s/%s.json", dir, json_website, json_num);
        JSON_ENTITY *ent = json_parse_file(json_path);
        htab_put(json_ht, key, &ent);
    }
    /* read_csv(dataset_X, csv, "0"); */
    read_csv(dataset_X, csv, "0");
    //printf("\n\n\n\n");
    //print_sts_diff(stdout, dataset_X);
    ml_create(&ml, sw);
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
        //print_vector(ml, vector);
    }
    sts_destroy(dataset_X);
    htab_free_entries(json_ht, (void (*)(void *)) free_json_ht_ent);
    free(json_ht);
    return 0;
}
