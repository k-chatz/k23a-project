/* file: spec_to_specs.h */
#include "../include/spec_to_specs.h"
#include <stdbool.h>


#define MAX_ID_LEN 128        /* assume that domain+spec_id is not 128 chars long */

/* #define PRINT_FILE "dot/sts_printout.dot" */
#define PRINT_FILE_VERBOSE true

#ifndef PRINT_FILE_VERBOSE
#define PRINT_FILE_VERBOSE false
#endif

#ifdef PRINT_FILE
FILE *PRINT_FILE_STREAM = NULL;
#endif

void print_spec(void *spec) {
    printf("spec_id = %s\n", ((SpecEntry *) spec)->id);
}

SpecEntry *findRoot(STS *sts, SpecEntry *spec) {
    SpecEntry *root = htab_get(sts->ht, spec->parent);
    if (spec == root)
        return spec;

    /* three shortening */
    spec->parent = ((SpecEntry *) htab_get(sts->ht, spec->parent))->parent;

    return findRoot(sts, root);
}

/* __________ STS Functions __________ */
/* Create a new sts */
STS *sts_new() {
    /* allocate the sts */
    STS *new = malloc(sizeof(STS));
    new->ht = htab_new(djb2_str, MAX_ID_LEN, sizeof(SpecEntry), 1999);
    new->ht->cmp = (ht_cmp_func) strncmp;
    new->ht->keycpy = (ht_key_cpy_func) strncpy;
    /* set the buffer */
#ifdef PRINT_FILE
    if(PRINT_FILE_STREAM == NULL)
    PRINT_FILE_STREAM = fopen(PRINT_FILE, "w+");
#endif
    return new;
}

/* Destroy a spec */
static void destroy_spec(SpecEntry *spec) {
    ll_free(spec->similar, free);
    free(spec->id);
}

void sts_destroy(STS *sts) {
    htab_destroy(sts->ht, (void (*)(void *)) destroy_spec);
    free(sts);
}

/* adds a node to the sts */
int sts_add(STS *sts, char *id) {
    bool rehash = false;
    int new_keysz = sts->ht->key_sz;
    int new_buf_cap = sts->ht->buf_cap;

    // Todo: inline function
    if ((((float) sts->ht->buf_load) / sts->ht->buf_cap) > 0.7) {
        new_buf_cap *= 2;
        rehash = true;
    }

    if (strlen(id) + 1 >= sts->ht->key_sz) {
	do {
	    new_keysz *= 2;	    
	} while(strlen(id) + 1 >= new_keysz);

        rehash = true;
    }

    if (rehash) {
        // Todo: change htab_rehash, execute below commands inside rehash
        hashp new_ht = htab_new(djb2_str, new_keysz, sizeof(SpecEntry), new_buf_cap);
        new_ht->keycpy = (ht_key_cpy_func) strncpy;
        new_ht->cmp = (ht_cmp_func) strncmp;
        htab_rehash(sts->ht, new_ht);
        free(sts->ht);
        sts->ht = new_ht;
    }
    SpecEntry temp = (SpecEntry) {};
    char *id_dup = strdup(id);
    temp.id = id_dup;
    htab_put(sts->ht, id, &temp);
    SpecEntry *newspec = htab_get(sts->ht, id);
    newspec->similar = malloc(sizeof(SpecList));
    newspec->similar->data = id_dup;
    newspec->similar->next = NULL;
    newspec->parent = id_dup;
    newspec->similar_tail = newspec->similar;
    newspec->similar_len = 1;
    return 0;
}

#ifdef PRINT_FILE
bool FIRST_RUN = true;
#endif

/* Merges two sts nodes to point to the same expanded list */
int sts_merge(STS *sts, char *id1, char *id2) {
#ifdef PRINT_FILE
    if(FIRST_RUN){
    print_sts(PRINT_FILE_STREAM, sts, PRINT_FILE_VERBOSE);
    FIRST_RUN = false;
    }
#endif
    SpecEntry *spec1, *spec2;
    spec1 = findRoot(sts, htab_get(sts->ht, id1));
    spec2 = findRoot(sts, htab_get(sts->ht, id2));

    if (spec1->similar_len < spec2->similar_len) {
        /* swap */
        SpecEntry *temp = spec1;
        spec1 = spec2;
        spec2 = temp;
    }

    if (spec1 == spec2)
        /* sets are already merged; nothing to do */
        return -1;

    spec1->similar_tail->next = spec2->similar;    /* append spec2->similar to spec1->similar */
    spec1->similar_tail = spec2->similar_tail;     /* set the new tail */
    spec1->similar_len += spec2->similar_len;     /* add the lengths */
    spec2->similar = NULL;
    spec2->similar_tail = NULL;

    spec2->parent = spec1->id;

#ifdef PRINT_FILE
    print_sts(PRINT_FILE_STREAM, sts, PRINT_FILE_VERBOSE);
#endif


    return 0;
}

SpecEntry *sts_get(STS *sts, char *id) { return htab_get(sts->ht, id); }


void free_StrList_data(StrList *list) {
    free(list->data);
    free(list);
}

void print_sts_dot(FILE *file, STS *sts, bool verbose) {
    fprintf(file, "digraph {\n\n");
    ulong iter_state = 0;
    for (char *key = htab_iterate_r(sts->ht, &iter_state); key != NULL; key = htab_iterate_r(sts->ht, &iter_state)) {
        SpecEntry *sp, *root;
        sp = htab_get(sts->ht, key);
        root = findRoot(sts, sp);
        StrList *similar = root->similar;

        if (!verbose || sp == root) {
            fprintf(file, "\"%s\" -> \"%p\"\n", sp->id, similar);
        } else {
            fprintf(file, "\"%s\" -> \"%s\"", sp->id, sp->parent);
        }

        char buf[500];

        if (root == sp) {
            switch (root->similar_len) {
                case 0:
                    strcpy(buf, "()");
                    break;
                case 1:
                    sprintf(buf, "(%s)", root->similar->data);
                    break;
                case 2:
                    sprintf(buf, "(%s, %s)", root->similar->data,
                            ((StrList *) ll_nth(root->similar, 1))->data);
                    break;
                case 3:
                    sprintf(buf, "(%s, %s, %s)", root->similar->data,
                            ((StrList *) ll_nth(root->similar, 1))->data,
                            ((StrList *) ll_nth(root->similar, 2))->data);
                    break;
                default:
                    sprintf(buf, "(%s, %s, %s, ...)", root->similar->data,
                            ((StrList *) ll_nth(root->similar, 1))->data,
                            ((StrList *) ll_nth(root->similar, 2))->data);
                    break;
            }
            fprintf(file, "\n\"%p\"[label=\"%s\"]\n", root->similar, buf);
        }

    }
    fprintf(file, "\n}\n");
}

// typedef struct list_s list;

// struct list_s {
//     list *next;
//     int data;
// };

bool eq_pred(void *node, va_list vargs) {
    char *x = va_arg(vargs, char*);
    StrList *n = (StrList *) node;
    return !strcmp(n->data, x);
}

StrList *create_node(char *id) {

    StrList *node = malloc(sizeof(StrList));
    node->data = strdup(id);
    node->next = NULL;
    return node;
}

void print_sts(FILE *file, STS *sts) {
    ulong iter_state = 0;
    for (char *key = htab_iterate_r(sts->ht, &iter_state);
         key != NULL;
         key = htab_iterate_r(sts->ht, &iter_state)) {
	/* get the spec */
        SpecEntry *sp = htab_get(sts->ht, key);
	/* is sp a representative? */
	if(sp == findRoot(sts, sp)){
	    /* sp is a set representative */
	    LLFOREACH(A, sp->similar){
		LLFOREACH(B, (StrList*)A->next){
		    fprintf(file, "%s, %s\n", A->data, B->data);
		}
	    }
	}
    }
}

/* _______ END of STS Functions _______ */
