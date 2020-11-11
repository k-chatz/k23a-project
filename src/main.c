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

int main(int argc, char *argv[]) {
    char left_spec_id[50], right_spec_id[50], label[50], *dir = NULL, *csv = NULL;
    FILE *fp;
    STS *dataset_X = NULL;

    readOptions(argc, argv, &dir, &csv);

    dataset_X = get_spec_ids(dir);
    fp = fopen(csv, "r");

    //skip first row fseek
    fseek(fp, 33, SEEK_SET);
    while (fscanf(fp, "%[^,],%[^,],%s\n", left_spec_id, right_spec_id, label) != EOF) {
        if (!strcmp(label, "1") && strcmp(left_spec_id, right_spec_id) != 0) {
            sts_merge(dataset_X, left_spec_id, right_spec_id);
        }
    }
    fclose(fp);
    
    //print result
    /* print_sts(dataset_X); */

    return 0;
}
