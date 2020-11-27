/* file: spec_to_specs.h */
#include "../include/spec_to_specs.h"
#include <stdbool.h>

#define MAX_ID_LEN 128 /* assume that domain+spec_id is not 128 chars long */

/* #define PRINT_FILE "dot/sts_printout.dot" */
#define PRINT_FILE_VERBOSE true

#ifndef PRINT_FILE_VERBOSE
#define PRINT_FILE_VERBOSE false
#endif

#ifdef PRINT_FILE
FILE *PRINT_FILE_STREAM = NULL;
#endif

void print_spec(void *spec) {
    printf("spec_id = %s\n", ((SpecEntry *)spec)->id);
}

SpecEntry *findRoot(STS *sts, SpecEntry *spec) {
    SpecEntry *root = dict_get(sts->ht, spec->parent);
    if (spec == root)
        return spec;

    /* three shortening */
    spec->parent = ((SpecEntry *)dict_get(sts->ht, spec->parent))->parent;

    return findRoot(sts, root);
}

/* __________ STS Functions __________ */
/* Create a new sts */
STS *sts_new() {
    /* allocate the sts */
    STS *new = malloc(sizeof(STS));
    /* clang-format off */
    /* @formatter:off */
    new->ht =
	dict_config
	(dict_new2(MAX_ID_LEN	, sizeof(SpecEntry)),
	 DICT_CONF_HASH_FUNC	, djb2_str,
	 DICT_CONF_KEY_CPY	, (ht_key_cpy_func)strncpy,
	 DICT_CONF_CMP		, (ht_cmp_func)strncmp,
	 DICT_CONF_KEY_SZ_F	, str_sz,
	 DICT_CONF_DONE);
    /* clang-format on */
    /* @formatter:on */
    /* set the buffer */
#ifdef PRINT_FILE
    if (PRINT_FILE_STREAM == NULL)
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
    dict_free(sts->ht, (void (*)(void *))destroy_spec);
    free(sts);
}

/* adds a node to the sts */
int sts_add(STS *sts, char *id) {
    SpecEntry temp;
    temp.id = strdup(id);
    temp.similar = malloc(sizeof(SpecList));
    temp.similar->data = temp.id;
    temp.similar->next = NULL;
    temp.parent = temp.id;
    temp.similar_tail = temp.similar;
    temp.similar_len = 1;

    dict_put(sts->ht, id, &temp);
    SpecEntry *newspec = dict_get(sts->ht, id);
    return 0;
}

#ifdef PRINT_FILE
bool FIRST_RUN = true;
#endif

/* Merges two sts nodes to point to the same expanded list */
int sts_merge(STS *sts, char *id1, char *id2) {
#ifdef PRINT_FILE
    if (FIRST_RUN) {
        print_sts(PRINT_FILE_STREAM, sts, PRINT_FILE_VERBOSE);
        FIRST_RUN = false;
    }
#endif
    SpecEntry *spec1, *spec2;
    spec1 = findRoot(sts, dict_get(sts->ht, id1));
    spec2 = findRoot(sts, dict_get(sts->ht, id2));

    if (spec1->similar_len < spec2->similar_len) {
        /* swap */
        SpecEntry *temp = spec1;
        spec1 = spec2;
        spec2 = temp;
    }

    if (spec1 == spec2)
        /* sets are already merged; nothing to do */
        return -1;

    spec1->similar_tail->next =
        spec2->similar; /* append spec2->similar to spec1->similar */
    spec1->similar_tail = spec2->similar_tail; /* set the new tail */
    spec1->similar_len += spec2->similar_len;  /* add the lengths */
    spec2->similar = NULL;
    spec2->similar_tail = NULL;

    spec2->parent = spec1->id;

#ifdef PRINT_FILE
    print_sts(PRINT_FILE_STREAM, sts, PRINT_FILE_VERBOSE);
#endif

    return 0;
}

SpecEntry *sts_get(STS *sts, char *id) { return dict_get(sts->ht, id); }

void free_StrList_data(StrList *list) {
    free(list->data);
    free(list);
}

void print_sts_dot(FILE *file, STS *sts, bool verbose) {
    fprintf(file, "digraph {\n\n");
    ulong iter_state = 0;
    for (char *key = dict_iterate_r(sts->ht, &iter_state); key != NULL;
         key = dict_iterate_r(sts->ht, &iter_state)) {
        SpecEntry *sp, *root;
        sp = dict_get(sts->ht, key);
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
                        ((StrList *)ll_nth(root->similar, 1))->data);
                break;
            case 3:
                sprintf(buf, "(%s, %s, %s)", root->similar->data,
                        ((StrList *)ll_nth(root->similar, 1))->data,
                        ((StrList *)ll_nth(root->similar, 2))->data);
                break;
            default:
                sprintf(buf, "(%s, %s, %s, ...)", root->similar->data,
                        ((StrList *)ll_nth(root->similar, 1))->data,
                        ((StrList *)ll_nth(root->similar, 2))->data);
                break;
            }
            fprintf(file, "\n\"%p\"[label=\"%s\"]\n", root->similar, buf);
        }
    }
    fprintf(file, "\n}\n");
}

bool eq_pred(void *node, va_list vargs) {
    char *x = va_arg(vargs, char *);
    StrList *n = (StrList *)node;
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
    for (char *key = dict_iterate_r(sts->ht, &iter_state); key != NULL;
         key = dict_iterate_r(sts->ht, &iter_state)) {
        /* get the spec */
        SpecEntry *sp = dict_get(sts->ht, key);
        /* is sp a representative? */
        if (sp == findRoot(sts, sp)) {
            /* sp is a set representative */
            LLFOREACH(A, sp->similar) {
                LLFOREACH(B, (StrList *)A->next) {
                    fprintf(file, "%s, %s\n", A->data, B->data);
                }
            }
        }
    }
}

/* _______ END of STS Functions _______ */
