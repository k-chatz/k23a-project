/* file: spec_to_specs.h */
#include "../include/spec_to_specs.h"
#include <stdbool.h>

#define MAX_ID_LEN 128		/* assume that domain+spec_id is not 128 chars long */


/* __________ STS Functions __________ */
/* Create a new sts */
STS *sts_new() {
     /* allocate the sts */
    STS *new = malloc(sizeof(STS));
    new->ht = htab_new(djb2_str, MAX_ID_LEN, sizeof(SpecEntry), 1999);
    new->ht->cmp = (ht_cmp_func)strncmp;
    new->ht->keycpy = (ht_key_cpy_func)strncpy;
    /* set the buffer */
    return new;
}

/* adds a node to the sts */

int sts_add(STS *sts, char *id) {
    bool rehash = false;
    int new_keysz = sts->ht->key_sz;
    int new_buf_cap = sts->ht->buf_cap;
    
    if((((float)sts->ht->buf_load) / sts->ht->buf_cap) > 0.7) {
	new_buf_cap *= 2;
	rehash = true;
    }

    if(strlen(id) > sts->ht->key_sz){
	new_keysz *= 2;
	rehash = true;
    }

    if(rehash){
	hashp new_ht = htab_new(djb2_str,
				new_keysz,
				sizeof(SpecEntry),
				new_buf_cap);
	new_ht->keycpy = (ht_key_cpy_func)strncpy;
	new_ht->cmp = (ht_cmp_func)strncmp;
	htab_rehash(sts->ht, new_ht);
	free(sts->ht);
	sts->ht = new_ht;
    }

    SpecEntry temp = (SpecEntry){};
    htab_put(sts->ht, id, &temp);
    SpecEntry *newspec = htab_get(sts->ht, id);

    newspec->similar = malloc(sizeof(SpecList));
    newspec->similar->data = strdup(id);
    newspec->similar->next = NULL;

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
	((SpecEntry*)htab_get(sts->ht, specEnt->data))->
	    similar = spec1->similar;
    }

    return 0;
}

SpecEntry *sts_get(STS *sts, char *id) { return htab_get(sts->ht, id); }

void print_sts(STS *sts) {
    // ht_print(sts->ht);

    ulong iter_state = 0;
    for(void *key = htab_iterate_r(sts->ht, &iter_state);
	key != NULL;
	key = htab_iterate_r(sts->ht, &iter_state)){
	SpecEntry *sp = htab_get(sts->ht, key);
        printf("%s -> (", key);
        StrList *similar = sp->similar;
        while (similar) {
            printf("%s ", similar->data);
            similar = ll_nth(similar, 1);
        }
        printf(")\n");
    }
}

/* _______ END of STS Functions _______ */
