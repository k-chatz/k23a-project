#include <stdio.h>
#include <stdlib.h>
#include "../include/lists.h"
#include "../include/spec_to_specs.h"
#include "../include/spec_ids.h"

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

/* Destroy a spec */
static void destroy_spec(SpecEntry *spec) {
    ll_free(spec->similar, free);
    free(spec->id);
}

int main(int argc, char *argv[]) {
   char left_spec_id[50], right_spec_id[50], label[50], *dir = NULL, *csv = NULL;
   FILE *fp;
   STS *dataset_X = NULL;
   readOptions(argc, argv, &dir, &csv);
   dataset_X = get_spec_ids(dir);
   // fp = fopen(csv, "r");
   // //skip first row fseek
   // fseek(fp, 33, SEEK_SET);
   // while (fscanf(fp, "%[^,],%[^,],%s\n", left_spec_id, right_spec_id, label) != EOF) {
   //     if (!strcmp(label, "1") && strcmp(left_spec_id, right_spec_id) != 0) {
   //         sts_merge(dataset_X, left_spec_id, right_spec_id);
   //     }
   // }
   // fclose(fp);
   //print result
   // print_sts_(stdout, dataset_X, false);
   // 12,894,598
   sts_destroy(dataset_X);

    // hashp ht;
    // ht = htab_new(djb2_str, 128, sizeof(SpecEntry), 1999);
    // ht->cmp = (ht_cmp_func) strncmp;
    // ht->keycpy = (ht_key_cpy_func) strncpy;

    // char id[128];
    // for(int i = 0 ; i < 1000; i++) {
    //     sprintf(id, "%d", i);
    //     SpecEntry spec = (SpecEntry) {};
    //     char *id_dup = strdup(id);
    //     spec.similar = malloc(sizeof(SpecList));
    //     spec.similar->data = id_dup;
    //     spec.similar->next = NULL;
    //     spec.parent = id_dup;
    //     spec.id = id_dup;
    //     spec.similar_tail = spec.similar;
    //     spec.similar_len = 1;
    //     //temp.id = strdup("id");
    //     htab_put(ht, spec.id, &spec);
    // }
    // htab_destroy(ht, (void (*)(void *)) destroy_spec);

    return 0;
}
