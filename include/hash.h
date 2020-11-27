#ifndef HASHTABLE_H
#define HASHTABLE_H

#include <assert.h>
#include <inttypes.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>

#define MAX_PROBES 32

/*!
@defgroup htab htab_t
@brief open addressed hashtables with random probing
@{
 */

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
    char contents[]; /* contains key, val */
} htab_entry_t;

/*!
@brief hashtable ADT with open addressing and random probing
*/
typedef struct htab_s {
    /*! @brief hash function used to hash the keys */
    ht_hash_func h;
    /*! @brief comparison function to compare 2 keys (default: memcmp) */
    ht_cmp_func cmp;
    /*! @brief copying function that copies a key to the hashtable (default:
     * memcpy) */
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

/*! @brief calculates the entry size for a hashtable with key_sz = key_sz and
 * val_sz = val_sz */
static inline size_t htab_entry_size2(size_t key_sz, size_t val_sz) {
    return sizeof(htab_entry_t) + key_sz + val_sz + sizeof(uintmax_t);
}

/*!
@relates htab_t
@brief calculates the entry size for ht
*/
static inline size_t htab_entry_size(htab_t *ht) {
    return htab_entry_size2(ht->key_sz, ht->val_sz);
}

/*! @brief calculates the total size of a hashtable, given capacity and key and
 * value sizes */
static inline size_t htab_size(ulong buf_cap, size_t key_sz, size_t val_sz) {
    return sizeof(htab_t) + htab_entry_size2(key_sz, val_sz) * buf_cap;
}

/*!
@relates htab_t
@brief initializes an allocated hashtable
@param[in] ht : the hash table
@param[in] ht_hash_func : the hashing function used for ht
@param[in] key_sz : size of keys
@param[in] val_sz : size of vals
*/
void htab_init(hashp ht, ht_hash_func h, size_t key_sz, size_t val_sz,
               ulong buf_cap);

/*!
@relates htab_t
@brief allocates and initializes a hashtable
(params same as htab_init)
 */
hashp htab_new(ht_hash_func h, size_t key_sz, size_t val_sz, ulong buf_cap);

/*!
@relates htab_t
@brief destroys a hash table
(for params see htab_free_entries)
 */
void htab_destroy(hashp ht, void (*free_t)(void *));

/*!
@relates htab_t
@brief frees all the occupied entries of a hashtable
@param[in] ht : the hash table
@param[in] free_t : a function to free the values with
 */
void htab_free_entries(hashp ht, void (*free_t)(void *));

/*!
@relates htab_t
@brief put an element to the hash table
@param[in] ht_info : the hash table
@param[in] key : the key to be added
@param[in] val : the value to be associated with key
*/
bool htab_put(hashp ht_info, keyp key, valp val);

/*!
@relates htab_t
@brief get an element from the hash table
@param[in] ht_info : the hash table
@param[in] key : the key to look for
@returns a pointer to the value associated with key. NULL if key is not in
ht_info
*/
valp htab_get(hashp ht_info, keyp key);

/*!
@relates htab_t
@brief from a value in the hash table, get the key.
@param[in] ht : the hash table
@param[in] val : the value
@returns a pointer to the key, val is associated with.
*/
const keyp htab_get_keyp_from_valp(hashp ht, valp val);

/*!
@relates htab_t
@brief get the internal copy of a key
@param[in] ht_info : the hash table
@param[in] key : the key
@returns If key is in ht_info, the equivelent of key stored in ht_info. NULL
otherwise.
 */
const keyp htab_get_keyp(hashp ht_info, keyp key);

/*!
@relates htab_t
@brief delete an entry from the hash table

Marks a key as deleted and returns the value associated with item

@param[in] ht_info : the hash table
@param[in] key : the key to be deleted
@returns the value associated with key
*/
valp htab_del(hashp ht_info, keyp key);

/*!
@relates htab_t
@brief puts all the keys of old to new
equivelent to htab_rehash_deep with the identity function
 */
bool htab_rehash(hashp old, hashp new);

/*!
@relates htab_t
@brief puts all the keys of old to new and copies associated values with
copy_val

@params[in] old : source hash table
@params[in] new : dest hash table
@params[in] copy_val : a function to copy the values with
 */
bool htab_rehash_deep(hashp old, hashp new, valp (*copy_val)(valp val));

/*!
@relates htab_t
@brief Iterates through the hashtable returning the next occupied element with
each call.

@params[in] ht : the hash table
@params[in] state : the state of the iterator
@returns the next key in the hash table
 */
void *htab_iterate_r(hashp ht, ulong *state);

/*!
@relates htab_t
@brief see htab_iterate_r
 */
void *htab_iterate(hashp ht);

/*!
@defgroup dict
@ingroup htab_t
@brief hashtables that live on the heap, in their own allocation
@{
 */
typedef struct {
    double max_load_factor;
    size_t (*key_sz)(keyp key);
    htab_t *htab;
} dict_t;

typedef dict_t *dictp;

/*! 
@brief create a new dict
 */
dictp dict_new2(size_t key_sz, size_t val_sz);
/*!
@brief create a new dict
@param[in] key_sz : the size of keys
@param[in] val_sz : the size of vals
@param[in] bufcap : the capacity of the buffer
@returns the new dict
*/
dictp dict_new3(size_t key_sz, size_t val_sz, ulong bufcap);

typedef
enum {
    /*! @brief ends the conf argument list */
    DICT_CONF_DONE,
    /*! @brief the argument after this is a hashing function */
    DICT_CONF_HASH_FUNC,
    /*! @brief the argument after this is a ht_key_cpy_func */
    DICT_CONF_KEY_CPY,
    /*! @brief the argument after this is a ht_cmp_func */
    DICT_CONF_CMP,
    /*! @brief the argument after this the max load factor for this dict */
    DICT_CONF_LF,
    /*! @brief the argument after this is a function that calculates a key's size */
    DICT_CONF_KEY_SZ_F,
} dict_conf_key;

/*!
@brief configures a dict

Accepts an ordered list of arguments of the form dict_conf_key, dict_conf_val, ...
the list is terminated by a DICT_CONF_DONE in the key position

@param[in] d : the dict
@param[in] ... : CONF_KEY, CONF_VAL, ..., CONF_DONE
@returns the dict
 */
dictp dict_config(dictp d, ...);
dictp dict_config_va(dictp d, va_list vargs);
/*!
@brief sets the hashing function for dict
 */
dictp dict_set_hfunc(dictp d, ht_hash_func f);
/*!
@brief sets the key copying method for d
 */
dictp dict_set_keycpy(dictp d, ht_key_cpy_func f);
/*!
@brief sets the key compare method for d
 */
dictp dict_set_cmp(dictp d, ht_cmp_func f);
/*!
@brief sets the target load factor for d
 */
dictp dict_set_max_load_factor(dictp d, double lf);
/*!
@brief sets the target load factor for d
 */
dictp dict_set_key_sz_f(dictp d, size_t (*f)(keyp key));
/*!
@brief put an element in dict
 */
dictp dict_put(dictp dict, keyp key, valp val);
/*!
@brief put many elements in dict
@param[in] dict : the dict
@param[out] num_put : the number of elements successfully put in dict
@param[in] ... : a null terminated series of key value pairs
 */
dictp dict_putv(dictp dict, int *num_put, ...);
/*!
@brief gen an element from dict
 */
valp dict_get(dictp dict, keyp key);

/*!
@brief force a rehash on d
 */
dictp dict_force_rehash2(dictp d, ulong new_bufcap);
/*!
@brief force a rehash on d with a new key size 
*/
dictp dict_force_rehash3(dictp d, ulong new_bufcap, size_t new_keysz);

/*!
@brief see htab_del
 */
valp dict_del(dictp d, keyp key);

/*!
@brief see htab_iterate
 */
keyp dict_iterate_r(dictp d, ulong *state);
keyp dict_iterate(dictp d);

/*!
@brief free the dict
see htab_destroy
 */
void dict_free(dictp dict, void (*free_t)(void *));

/* __________ Some utility functions __________ */

/*!
@brief get size of string key
 */
size_t str_sz(keyp key);

/*! @} */

/* __________ Some hashing functions to use __________ */
/*! @brief djb2 hashing function */
uint djb2(keyp key, size_t key_sz);

/*! @brief djb2 hashing function for strings */
uint djb2_str(keyp key, size_t key_sz);


/*! @} */

#endif
