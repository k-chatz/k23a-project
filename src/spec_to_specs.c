/* file: spec_to_specs.h */
#include "../include/spec_to_specs.h"

/* Created a spec node to be added to the hash table */
static void *create_spec(void *id) {
    SpecEntry *new = malloc(sizeof(SpecEntry));
    new->id = strdup(id);
    new->similar = malloc(sizeof(SpecList));
    new->similar->next = NULL;
    new->similar->data = new;    /* add self to list of similar specs */
    return new;
}

void print_spec(void *spec) {
    printf("spec_id = %s\n", ((SpecEntry *) spec)->id);
}

/* Hash an id */
static ulong hash_spec(void *id, ulong htcap) {
    ulong sum = 0;
    while (*(char *) id) {
        sum *= 47;            /* multiply by a prime number */
        sum += *((char *) id);
        id++;
    }
    return sum % htcap;
}

/* Compare a spec to a key */
static int cmp_spec(void *spec, void *key) {
    return strcmp(((SpecEntry *) spec)->id, key);
}

/* Destroy a spec */
static ulong destroy_spec(void *spec) {
    SpecList **similarp = &(((SpecEntry *) spec)->similar);
    while (*similarp) {
        if (strcmp(((SpecEntry *) spec)->id, (*similarp)->data->id) == 0) {
            /* remove this element */
            SpecList *poped = ll_pop(similarp);
            free(poped);
        }
        similarp = ll_nth(similarp, 1);
    }
    free(((SpecEntry *) spec)->id);
    free(spec);
    return 1;
}


/* __________ STS Functions __________ */
/* Create a new sts */
STS *sts_new() {
    STS *new = malloc(sizeof(STS));
    ht_init(&(new->ht),
            HT_CAP,
            HT_BSZ,
            create_spec,
            cmp_spec,
            print_spec,
            hash_spec,
            destroy_spec);
    new->keys = NULL;
    return new;
}

/* adds a node to the sts */
int sts_add(STS *sts, char *id) {
    SpecEntry specEntry;
    StrList *new_id = malloc(sizeof(StrList));
    new_id->data = strdup(id);
    ll_push(&(sts->keys), new_id);
    ht_insert(sts->ht, id, id, (void **) &specEntry);
    return 0;
}

/* Merges two sts nodes to point to the same expanded list */
int sts_merge(STS *sts, char *id1, char *id2) {
    SpecEntry *spec1, *spec2;
    spec1 = ht_get(sts->ht, id1);
    spec2 = ht_get(sts->ht, id2);

    if (spec1->similar == spec2->similar)
        return 0;

    SpecList *spec2_similar = spec2->similar;
    SpecList *iter;

    while (spec2_similar) {
        ll_push(&(spec1->similar), ll_pop(&spec2_similar));
    }

    for (iter = spec1->similar; iter; iter = ll_nth(iter, 1)) {
        iter->data->similar = spec1->similar;
    }

    return 0;
}

SpecEntry *sts_get(STS *sts, char *id) {
    return ht_get(sts->ht, id);
}

void print_sts(STS *sts) {
    //ht_print(sts->ht);

    StrList *keys = sts->keys;
    while (keys) {
        SpecEntry *sp = ht_get(sts->ht, keys->data);
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
