#ifndef HASHTABLE_H
#define HASHTABLE_H

#include <stdbool.h>
#include <stdio.h>

typedef void *pointer;

typedef struct Hashtable *Hashtable;

bool ht_init(
        Hashtable *ht,
        unsigned long capacity,
        unsigned long int bucketSize,
        pointer(*createValue)(pointer),
        int (*cmp)(pointer, pointer),
        void (*printValue)(pointer),
        unsigned long (*hash)(pointer, unsigned long int),
        unsigned long (*destroy)(pointer)
);

int ht_insert(Hashtable ht, pointer key, pointer valueParams, void **value);

pointer ht_get(Hashtable ht, pointer key);

int ht_remove(Hashtable ht, pointer key, pointer valueParams, bool forceDestroyItem);

void ht_destroy(Hashtable *ht, bool forceDestroyItem);

void ht_print(Hashtable ht);

#endif
