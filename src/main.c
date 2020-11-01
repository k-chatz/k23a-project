#include <stdio.h>
#include <stdlib.h>
#include "../include/lists.h"
#include "../include/spec_to_specs.h"
#include "../include/spec_ids.h"

typedef struct {
    void *next;
    int data;
} intList;

intList *new_node(int data) {
    intList *out = malloc(sizeof(intList));
    out->data = data;
    return out;
}

void readOptions(int argc,char **argv,char **folder) {
    int i;
    char *opt, *optVal;
    for (i = 1; i < argc; ++i) {
        opt = argv[i];
        optVal = argv[i + 1];
        if (strcmp(opt, "-f") == 0) {
            if (optVal != NULL && optVal[0] != '-') {
                *folder = optVal;
            }
        }
    }
}

int main(int argc, char *argv[]) {
    int list_elements[] = {1, 2, 3, 4, 5, 6}, i = 0;
    char * folder = NULL;
    readOptions(argc, argv, &folder);

    intList *L = NULL;
    for (i = 0; i < 6; i++) {
        ll_push(&L, new_node(list_elements[i]));
    }

    intList *temp = L;
    printf("(");
    while (temp) {
        printf("%d ", temp->data);
        temp = ll_nth(temp, 1);
    }
    printf(")\n");

    ll_free(temp, free);

    STS *result = get_spec_ids(folder);

    //print result
    print_sts(result);

    return 0;
}
