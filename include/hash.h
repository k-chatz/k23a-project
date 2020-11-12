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

/*! @brief hashtable bucket states */
enum {
  HT_ENTRY_FLAGS_CLEAR = 0,
  HT_ENTRY_FLAGS_OCCUPIED = 0b1,
  HT_ENTRY_FLAGS_DELETED = 0b10
};

/*! @brief hashtable entry struct */
typedef struct htab_entry_s {
  u_int8_t flags;
  /*!  */
  uint hash;
  /*! 
    @brief the buffer for the entry 
    shold be of size htab_s.buf_cap * htab_entry_sz(htab_s)
   */
  char contents[]; /* contains hash, key, val */
} htab_entry_t;

/*! hashtable ADT */
typedef struct htab_s {
  /*! @brief hash function used to hash the keys */
  ht_hash_func h;
  /*! @brief comparison function to compare 2 keys (default: memcmp) */
  ht_cmp_func cmp;
  /*! @brief copying function that copies a key to the hashtable (default: memcpy) */
  ht_key_cpy_func keycpy;
  /*! @brief size of key in the hashtable */
  size_t key_sz;
  /*! @brief size of val in the hashtable */
  size_t val_sz;
  /*! @brief capacity of buf */
  ulong buf_cap;
  /*! @brief occupied entries of buf */
  ulong buf_load;
  /*! @brief the buffer where the entries are stored */
  char buf[];
} htab_t;

typedef htab_t *hashp;

/*! @brief calculates the entry size for a hashtable with key_sz = key_sz and val_sz = val_sz */
static inline size_t htab_entry_size2(size_t key_sz, size_t val_sz) {
  return sizeof(htab_entry_t) + key_sz + val_sz + sizeof(uintmax_t);
}

/*! @brief calculates the entry size for ht */
static inline size_t htab_entry_size(hashp ht) {
  return htab_entry_size2(ht->key_sz, ht->val_sz);
}

/*! @brief calculates the total size of a hashtable, given capacity and key and value sizes */
static inline size_t htab_size(ulong buf_cap, size_t key_sz, size_t val_sz) {
  return sizeof(htab_t) + htab_entry_size2(key_sz, val_sz) * buf_cap;
}

void htab_init(hashp ht, ht_hash_func h, size_t key_sz, size_t val_sz,
               ulong buf_cap);
hashp htab_new(ht_hash_func h, size_t key_sz, size_t val_sz, ulong buf_cap);
void htab_free_entries(hashp ht, void (*free)(void *));
bool htab_put(hashp ht_info, keyp key, valp val);
valp htab_get(hashp ht_info, keyp key);
const keyp htab_get_keyp_from_valp(hashp ht, valp val);
const keyp htab_get_keyp(hashp ht_info, keyp key);
valp htab_del(hashp ht_info, keyp key);
bool htab_rehash(hashp dest, hashp src);
bool htab_rehash_deep(hashp old, hashp new, valp (*copy_val)(valp val));
void *htab_iterate_r(hashp ht, ulong *state);
void *htab_iterate(hashp ht);
  
/* __________ Some hashing functions to use __________ */
uint djb2(keyp key, size_t key_sz);
uint djb2_str(keyp key, size_t key_sz);

#endif
