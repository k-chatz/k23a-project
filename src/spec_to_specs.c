/* file: spec_to_specs.h */
#include "../include/spec_to_specs.h"
#include <stdbool.h>

/* #define PRINT_FILE "dot/sts_printout.dot" */
#define PRINT_FILE_VERBOSE true

#ifndef PRINT_FILE_VERBOSE
#define PRINT_FILE_VERBOSE false
#endif

#ifdef PRINT_FILE
FILE *PRINT_FILE_STREAM = NULL;
#endif

/* Created a spec node to be added to the hash table */
static void *create_spec(void *id) {
    SpecEntry *new = malloc(sizeof(SpecEntry));
    new->id = strdup(id);
    new->parent = new;
    new->similar = malloc(sizeof(SpecList));
    new->similar->data = new;
    new->similar->next = NULL;
    new->similar_tail = new->similar;
    new->similar_len = 1;
    return new;
}

void print_spec(void *spec) {
    printf("spec_id = %s\n", ((SpecEntry *)spec)->id);
}

SpecEntry *findRoot(SpecEntry *spec){
    SpecEntry *root = spec->parent;
    if(spec == root)
	return spec;

    /* three shortening */
    spec->parent = spec->parent->parent;

    return findRoot(root);
}

/* Hash an id */
static ulong hash_spec(void *id, ulong htcap) {
    ulong sum = 0;
    while (*(char *)id) {
        sum *= 47; /* multiply by a prime number */
        sum += *((char *)id);
        id++;
    }
    return sum % htcap;
}

/* Compare a spec to a key */
static int cmp_spec(void *spec, void *key) {
    return strcmp(((SpecEntry *)spec)->id, key);
}

/* Destroy a spec */
static ulong destroy_spec(void *spec) {
    ll_free(((SpecEntry*)spec)->similar, free);
    free(((SpecEntry *)spec)->id);
    free(spec);
    return 1;
}

/* __________ STS Functions __________ */
/* Create a new sts */
STS *sts_new() {
    STS *new = malloc(sizeof(STS));
    ht_init(&(new->ht), HT_CAP, HT_BSZ, create_spec, cmp_spec, print_spec,
            hash_spec, destroy_spec);
    new->keys = NULL;
#ifdef PRINT_FILE
    if(PRINT_FILE_STREAM == NULL)
	PRINT_FILE_STREAM = fopen(PRINT_FILE, "w");
#endif

    return new;
}

/* adds a node to the sts */
int sts_add(STS *sts, char *id) {
    SpecEntry specEntry;
    StrList *new_id = malloc(sizeof(StrList));
    new_id->data = strdup(id);
    ll_push(&(sts->keys), new_id);
    ht_insert(sts->ht, id, id, (void **)&specEntry);
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
    spec1 = findRoot(ht_get(sts->ht, id1));
    spec2 = findRoot(ht_get(sts->ht, id2));

    if(spec1->similar_len < spec2->similar_len){
	/* swap */
	SpecEntry *temp = spec1;
	spec1 = spec2;
	spec2 = temp;
    }

   if (spec1 == spec2)
	/* sets are already merged; nothing to do */
        return 0;

   ll_push(spec1->similar_tail, spec2->similar); /* append spec2->similar to spec1->similar */
   spec1->similar_tail = spec2->similar_tail;	 /* set the new tail */
   spec1->similar_len += spec2->similar_len;	 /* add the lengths */
   spec2->similar = NULL;
   spec2->similar_tail = NULL;

   spec2->parent = spec1;

#ifdef PRINT_FILE
   print_sts(PRINT_FILE_STREAM, sts, PRINT_FILE_VERBOSE);
#endif    


   return 0;
}

SpecEntry *sts_get(STS *sts, char *id) { return ht_get(sts->ht, id); }

void print_sts(FILE *file, STS *sts, bool verbose) {
    fprintf(file, "digraph {\n\n");
    StrList *keys = sts->keys;
    LLFOREACH(key, keys) {
        SpecEntry *sp, *root;
	sp = ht_get(sts->ht, key->data);
	root = findRoot(sp);
        SpecList *similar = root->similar;

	if(verbose || sp == root){
	    fprintf(file, "\"%s\" -> \"%p\"\n", sp->id, similar);
	} else {
	    fprintf(file, "\"%s\" -> \"%s\"", sp->id, sp->parent->id);
	}
	
	char buf[500];

	if(root == sp){
          switch (root->similar_len) {
          case 0:
            strcpy(buf, "()");
            break;
          case 1:
            sprintf(buf, "(%s)", root->similar->data->id);
            break;
          case 2:
            sprintf(buf, "(%s, %s)", root->similar->data->id,
                    ((SpecList *)ll_nth(root->similar, 1))->data->id);
            break;
          case 3:
            sprintf(buf, "(%s, %s, %s)", root->similar->data->id,
                    ((SpecList *)ll_nth(root->similar, 1))->data->id,
                    ((SpecList *)ll_nth(root->similar, 2))->data->id);
            break;
          default:
            sprintf(buf, "(%s, %s, %s, ...)", root->similar->data->id,
                    ((SpecList *)ll_nth(root->similar, 1))->data->id,
                    ((SpecList *)ll_nth(root->similar, 2))->data->id);
            break;
          }
          fprintf(file, "\n\"%p\"[label=\"%s\"]\n", root->similar, buf);
        }
    }
    fprintf(file, "\n}\n");
}
/* _______ END of STS Functions _______ */
