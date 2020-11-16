#include <stdio.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>

#include "../include/lists.h"
#include "../include/spec_to_specs.h"
#include "../include/spec_ids.h"
#include "../include/json_parser.h"

void readOptions(int argc, char **argv, char **dir, char **csv) {
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
        }
    }
}

void free_json_ht_ent(JSON_ENTITY **val){
  json_entity_free(*val);
}

int main(int argc, char *argv[]) {
    char left_spec_id[50], right_spec_id[50], label[50], *dir = NULL, *csv = NULL;
    FILE *fp;
    STS *dataset_X = NULL;
    readOptions(argc, argv, &dir, &csv);
    dataset_X = get_spec_ids(dir);

    fp = fopen(csv, "r");
    //skip first row fseek
    uint merges = 0;
    fseek(fp, 33, SEEK_SET);
    while (fscanf(fp, "%[^,],%[^,],%s\n", left_spec_id, right_spec_id, label) != EOF) {
        if (!strcmp(label, "1") && strcmp(left_spec_id, right_spec_id) != 0) {
            merges += sts_merge(dataset_X, left_spec_id, right_spec_id) >= 0;
        }
    }
    fclose(fp);

    //print result
    print_sts(stdout, dataset_X, false);
    int fd;
    char json_website[128], json_num[128], json_path[280], *contents, *rest_tok;
    int read_err = 0;
    contents = malloc(1 << 20);
    hashp json_ht = htab_new(djb2_str, 128, sizeof(JSON_ENTITY *), dataset_X->ht->buf_cap);
    ulong iter_state = 0;
    for (char *key = htab_iterate_r(dataset_X->ht, &iter_state);
         key != NULL; key = htab_iterate_r(dataset_X->ht, &iter_state)) {
        sscanf(key, "%[^/]//%[^/]", json_website, json_num);
        snprintf(json_path, 280, "%s/%s/%s.json", dir, json_website, json_num);
        fd = open(json_path, O_RDONLY);
        read_err = read(fd, contents, 1 << 20);
	if(read_err < 0) {
	  perror("read");
	} else {
	  contents[read_err] = '\0';
	}
        StringList *tokens = json_tokenize_str(contents, &rest_tok);
        if (rest_tok[0] != '\0') {
            printf("rest not null");
            goto label;
        }
	memset(contents, 0, 1 << 20);
        StringList *rest_ent;
        JSON_ENTITY *ent = json_parse_value(tokens, &rest_ent);
        htab_put(json_ht, key, &ent);
        // empty tokens list
        label:
        ll_free(tokens, (llfree_f) json_free_StringList);
        close(fd);
    }
    sts_destroy(dataset_X);
    htab_free_entries(json_ht, (void (*)(void*))free_json_ht_ent);
    free(json_ht);
    free(contents);
    return 0;
}
