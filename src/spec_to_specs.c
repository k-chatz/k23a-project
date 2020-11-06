/* file: spec_to_specs.h */
#include "../include/spec_to_specs.h"
#include <stdbool.h>

#define MAX_ID_LEN 128		/* assume that domain+spec_id is not 128 chars long */

/* Created a spec node to be added to the hash table */
static SpecEntry *create_spec(char *id) {
    SpecEntry *new = malloc(sizeof(SpecEntry));
    new->id = strdup(id);
    new->similar = malloc(sizeof(SpecList));
    new->similar->next = NULL;
    new->similar->data = new; /* add self to list of similar specs */
    return new;
}

/* __________ STS Functions __________ */
/* Create a new sts */
STS *sts_new() {
     /* allocate the sts */
    STS *new = malloc(sizeof(STS));
    new->ht = htab_new(djb2_str, MAX_ID_LEN, sizeof(SpecEntry), 1999);
    new->ht->cmp = (ht_cmp_func)strncmp;
    new->ht->keycpy = (ht_key_cpy_func)strncpy;
    new->keys = NULL;
    /* set the buffer */
    return new;
}

/* adds a node to the sts */

int sts_add(STS *sts, char *id) {
    SpecEntry *newspec = create_spec(id);
    StrList *new_id = malloc(sizeof(StrList));
    new_id->data = strdup(id);
    ll_push(&(sts->keys), new_id);
    htab_put(sts->ht, id, newspec);
    free(newspec);
    return 0;
}

/* Merges two sts nodes to point to the same expanded list */
int sts_merge(STS *sts, char *id1, char *id2) {
    SpecEntry *spec1, *spec2;
    spec1 = htab_get(sts->ht, id1);
    spec2 = htab_get(sts->ht, id2);

    if (spec1->similar == spec2->similar)
	/* sets are already merged; nothing to do */
        return 0;

    ll_pushlist(&spec1->similar, spec2->similar);

    LLFOREACH(specEnt, spec1->similar){
	specEnt->data->similar = spec1->similar;
    }

    return 0;
}

SpecEntry *sts_get(STS *sts, char *id) { return htab_get(sts->ht, id); }

void print_sts(STS *sts) {
    // ht_print(sts->ht);

    StrList *keys = sts->keys;
    while (keys) {
        SpecEntry *sp = htab_get(sts->ht, keys->data);
        printf("%s -> (", sp->id);
        SpecList *similar = sp->similar;
        while (similar) {
            printf("%s ", similar->data->id);
            similar = ll_nth(similar, 1);
        }
        printf(")\n");
        keys = ll_nth(keys, 1);
    }
}
/* _______ END of STS Functions _______ */
