#ifndef HASHTABLE_H
#define HASHTABLE_H
#include <assert.h>
#include <inttypes.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define MAX_PROBES 32

typedef void *keyp;
typedef void *valp;

typedef uint (*ht_hash_func)(keyp key, size_t key_sz);
typedef int (*ht_cmp_func)(keyp key1, keyp key2, size_t keysz);
typedef void (*ht_key_cpy_func)(keyp dest, keyp src, size_t keysz);

enum {
    HT_ENTRY_FLAGS_CLEAR = 0,
    HT_ENTRY_FLAGS_OCCUPIED = 0b1,
    HT_ENTRY_FLAGS_DELETED = 0b10
};

typedef struct htab_entry_s {
    u_int8_t flags;
    uint hash;
    char contents[]; /* contains hash, key, val */
} htab_entry_t;

typedef struct htab_s {
    ht_hash_func h;
    ht_cmp_func cmp;
    ht_key_cpy_func keycpy;
    size_t key_sz;
    size_t val_sz;
    ulong buf_cap;
    ulong buf_load;
    char buf[];
} htab_t;

typedef htab_t *hashp;

static inline size_t htab_entry_size2(size_t key_sz, size_t val_sz) {
    return sizeof(htab_entry_t) + key_sz + val_sz + sizeof(uintmax_t);
}

static inline size_t htab_entry_size(hashp ht) {
    return htab_entry_size2(ht->key_sz, ht->val_sz);
}

static inline size_t htab_size(ulong buf_cap, size_t key_sz, size_t val_sz){
    return sizeof(htab_t) + htab_entry_size2(key_sz, val_sz) * buf_cap;
}


void htab_init(hashp ht, ht_hash_func h, size_t key_sz, size_t val_sz, ulong buf_cap);
hashp htab_new(ht_hash_func h, size_t key_sz, size_t val_sz, ulong buf_cap);
void htab_free_entries(hashp ht, void (*free)(void *));
bool htab_put(hashp ht_info, keyp key, valp val);
valp htab_get(hashp ht_info, keyp key);
valp htab_del(hashp ht_info, keyp key);
bool htab_copy(hashp dest, hashp src);
bool htab_rehash_deep(hashp old, hashp new, valp (*copy_val)(valp val));

/* __________ Some hashing functions to use __________ */
uint djb2(keyp key, size_t key_sz);
uint djb2_str(keyp key, size_t key_sz);

#endif
