/* file: spec_to_specs.h */
#include "../include/spec_to_specs.h"
#include "../include/lists.h"
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

bool eq_pred(void *node, va_list vargs) {
    char *x = va_arg(vargs, char *);
    StrList *n = (StrList *) node;
    return !strcmp(n->data, x);
}

StrList *create_node(char *id) {
    StrList *node = malloc(sizeof(StrList));
    node->data = id;
    node->next = NULL;
    return node;
}

void print_spec(void *spec) {
    printf("spec_id = %s\n", ((SpecEntry *) spec)->id);
}

SpecEntry *findRoot(STS *sts, SpecEntry *spec) {
    SpecEntry *root = dict_get(sts->dict, spec->parent);
    if (spec == root)
        return spec;

    /* three shortening */
    spec->parent = ((SpecEntry *) dict_get(sts->dict, spec->parent))->parent;

    return findRoot(sts, root);
}

/* __________ STS Functions __________ */
/* Create a new sts */
STS *sts_new() {
    /* allocate the sts */
    STS *new = malloc(sizeof(STS));
    /* clang-format off */
    /* @formatter:off */
    new->dict = dict_config(
            dict_new2(MAX_ID_LEN, sizeof(SpecEntry)),
            DICT_CONF_HASH_FUNC, djb2_str,
            DICT_CONF_KEY_CPY, (ht_key_cpy_func) strncpy,
            DICT_CONF_CMP, (ht_cmp_func) strncmp,
            DICT_CONF_KEY_SZ_F, str_sz,
            DICT_CONF_DONE
    );
    /* clang-format on */
    /* @formatter:on */
    /* set the buffer */
#ifdef PRINT_FILE
    if (PRINT_FILE_STREAM == NULL)
        PRINT_FILE_STREAM = fopen(PRINT_FILE, "w+");
#endif
    return new;
}

void free_spec_list_node(StrList *node) {
    free(node->data);
    free(node);
}

/* Destroy a spec */
static void destroy_spec(SpecEntry *spec) {
    free(spec->id);
    ll_free(spec->similar, (llfree_f) free);
    ll_free(spec->different, (llfree_f) free);
}

void sts_destroy(STS *sts) {
    dict_free(sts->dict, (void (*)(void *)) destroy_spec);
    free(sts);
}

/* adds a node to the sts */
int sts_add(STS *sts, char *id) {
    SpecEntry temp;
    temp.id = strdup(id);
    temp.similar = malloc(sizeof(StrList));
    temp.similar->data = temp.id;
    temp.similar->next = NULL;
    temp.parent = temp.id;
    temp.similar_tail = temp.similar;
    temp.similar_len = 1;
    temp.different = NULL;
    temp.different_len = 0;
    temp.printed = false;
    dict_put(sts->dict, id, &temp);
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
    spec1 = findRoot(sts, dict_get(sts->dict, id1));
    spec2 = findRoot(sts, dict_get(sts->dict, id2));

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

/* Merges two sts nodes to point to the same expanded list */
int sts_diff(STS *sts, char *id1, char *id2) {
    SpecEntry *spec1, *spec2;
    spec1 = findRoot(sts, dict_get(sts->dict, id1));
    spec2 = findRoot(sts, dict_get(sts->dict, id2));

    if (spec1->different_len < spec2->different_len) {
        /* swap */
        SpecEntry *temp = spec1;
        spec1 = spec2;
        spec2 = temp;
    }

    if (spec1->different == NULL) {
        spec1->different = malloc(sizeof(StrList));
        spec1->different->data = spec2->id;
        spec1->different->next = NULL;
        spec1->different_tail = spec1->different;
        spec1->different_len = 1;
    } else {
        StrList *result = ll_search(spec1->different, &eq_pred, spec2->id);
        if (result == NULL) {
            ll_push(spec1->different, create_node(spec2->id));
        }
    }

    if (spec2->different == NULL) {
        spec2->different = malloc(sizeof(StrList));
        spec2->different->data = spec1->id;
        spec2->different->next = NULL;
        spec2->different_tail = spec2->different;
        spec2->different_len = 1;
    } else {
        StrList *result = ll_search(spec2->different, &eq_pred, spec1->id);
        if (result == NULL) {
            ll_push(spec2->different, create_node(spec1->id));
        }
    }
#ifdef PRINT_FILE
    print_sts(PRINT_FILE_STREAM, sts, PRINT_FILE_VERBOSE);
#endif

    return 0;
}

SpecEntry *sts_get(STS *sts, char *id) {
    return dict_get(sts->dict, id);
}

void free_StrList_data(StrList *list) {
    free(list->data);
    free(list);
}

void print_sts_dot(FILE *file, STS *sts, bool verbose) {
    fprintf(file, "digraph {\n\n");
    ulong iter_state = 0;
    for (char *key = dict_iterate_r(sts->dict, &iter_state); key != NULL;
         key = dict_iterate_r(sts->dict, &iter_state)) {
        SpecEntry *sp, *root;
        sp = dict_get(sts->dict, key);
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

void init_similar_pairs(FILE *file, STS *sts, Pair **pairs, int *chunks, int *counter) {
    ulong iter_state = 0;
    char *entry = NULL;
    DICT_FOREACH_ENTRY(entry, sts->dict, &iter_state, sts->dict->htab->buf_load) {
        /* get the spec */
        SpecEntry *sp = dict_get(sts->dict, entry);
        /* is sp a representative? */
        if (sp == findRoot(sts, sp)) {
            /* sp is a set representative */
            LL_FOREACH(A, sp->similar) {
                LL_FOREACH(B, (StrList *) A->next) {
                    if ((*counter) + 1 > (*chunks * MATCHES_CHUNK_SIZE)) {
                        (*chunks)++;
                        *pairs = realloc(*pairs, (*chunks) * MATCHES_CHUNK_SIZE * sizeof(struct pair));
                    }
                    //fprintf(file, "%s, %s\n", A->data, B->data);
                    (*pairs)[*counter].spec1 = A->data;
                    (*pairs)[*counter].spec2 = B->data;
                    (*pairs)[*counter].relation = 1;
                    (*counter)++;
                }
            }
        }
    }
}

void init_different_pairs(FILE *file, STS *sts, Pair **pairs, int *chunks, int *counter) {
    ulong iter_state = 0;
    char *entry = NULL;
    DICT_FOREACH_ENTRY(entry, sts->dict, &iter_state, sts->dict->htab->buf_load) {
        /* get the spec */
        SpecEntry *sp = dict_get(sts->dict, entry), *temp;
        /* is sp a representative? */
        if (sp == findRoot(sts, sp)) {
            /* sp is a set representative */
            LL_FOREACH(A, sp->different) {
                temp = dict_get(sts->dict, A->data);
                if (temp->printed) {
                    LL_FOREACH(A_sim, (StrList *) A->next) {
                        LL_FOREACH(B, temp->similar) {
                            if ((*counter) + 1 > (*chunks * MATCHES_CHUNK_SIZE)) {
                                (*chunks)++;
                                (*pairs) = realloc(*pairs, (*chunks) * MATCHES_CHUNK_SIZE * sizeof(struct pair));
                            }
                            //fprintf(file, "%s, %s\n", A_sim->data, B->data);
                            (*pairs)[*counter].spec1 = A_sim->data;
                            (*pairs)[*counter].spec2 = B->data;
                            (*pairs)[*counter].relation = 0;
                            (*counter)++;
                        }
                    }
                }
            }
            sp->printed = true;
        }
    }
}

void print_sts_similar(FILE *file, STS *sts) {
    ulong iter_state = 0;
    char *entry = NULL;
    DICT_FOREACH_ENTRY(entry, sts->dict, &iter_state, sts->dict->htab->buf_load) {
        SpecEntry *sp, *root;
        sp = dict_get(sts->dict, entry);
        root = findRoot(sts, sp);
        if (root->id != sp->id) {
            continue;
        }

        for (StrList *similar = root->similar; similar; similar = ll_nth(similar, 1)) {
            printf("%s%s", similar == root->similar ? "" : ", ", similar->data);
        }
        printf("\n");
    }
}

void print_sts_diff(FILE *file, STS *sts) {
    ulong iter_state = 0;
    char *entry = NULL;
    DICT_FOREACH_ENTRY(entry, sts->dict, &iter_state, sts->dict->htab->buf_load) {
        SpecEntry *sp, *root;
        sp = dict_get(sts->dict, entry);
        root = findRoot(sts, sp);
        if (root->id != sp->id) {
            continue;
        }
        for (StrList *diff = root->different; diff; diff = ll_nth(diff, 1)) {
            printf("%s%s", diff == root->different ? "" : ", ", diff->data);
        }
        printf("\n");
    }
}


/* _______ END of STS Functions _______ */
