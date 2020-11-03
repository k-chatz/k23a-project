#ifndef HASHTABLE_H
#define HASHTABLE_H

#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>

typedef void *pointer;

typedef struct Hashtable *Hashtable;

/*!
@brief key pointer type
@showinitializer
*/
typedef void *keyp;
/*!
@brief val pointer type
@showinitializer
*/
typedef void *valp;

/*! @brief hashtable value contructor */
typedef valp (*hash_create_val_f)(valp);
/*! @brief hashtable comparator */
typedef int (*hash_compare_f)(valp, keyp);
/*! @brief hashtable printing function */
typedef void (*hash_print_value_f)(valp);
/*! @brief hashtable hashing function */
typedef ulong (*hash_hashfunc_f)(keyp, ulong);
/*! @brief hashtable value destructor */
typedef ulong (*hash_destroy_value)(valp);

/*!
@relates Hashtable
@brief Create a new hashtable.
@param[out] ht : The new Hashtable.
@param[in] capacity : The capacity of the hashtable's buffer.
@param[in] bucketSize : The capacity of a bucket.
@param[in] createValue : value contructor.
@param[in] cmp : value comparator.
@param[in] printValue : value printer.
@param[in] hash : hash function to be used.
@param[in] destroy : value destruction function to be used.
@returns returns TRUE on success
*/
bool ht_init(Hashtable *ht, unsigned long capacity,
             unsigned long int bucketSize, hash_create_val_f createValue,
             hash_compare_f cmp, hash_print_value_f printValue,
             hash_hashfunc_f hash, hash_destroy_value destroy);

/*!
@relates Hashtable
@brief Inserts a new (key, value) pair to the hashtable.
@param[in] ht : the hashtable
@param[in] key : the new key
@param[in] val : the new value
@param[out] value_out : the newly inserted value
@returns non-zero on success
 */
int ht_insert(Hashtable ht, keyp key, valp val, valp *value_out);

/*!
@relates Hashtable
@brief Finds a key on the hashtable
@param[in] ht : the hashtable
@param[in] key : the key to search for
@returns pointer to the value associated with key. NULL on failure*/
valp ht_get(Hashtable ht, keyp key);

/*!
@relates Hashtable
@brief Deletes a key, value pair from the hashtable
@param[in] ht : the hashtable
@param[in] key : the key
@param[in] valueParams : the value
@param[in] forcedestroyitem : whether to free the deleted item using ht->destroy
@returns non-zero on success
*/
int ht_remove(Hashtable ht, keyp key, valp valueParams, bool forceDestroyItem);

/*!
@relates Hashtable
@brief Destroys the hashtable, optionally freeing the items in it
@param[in] ht : the hash table
@param[in] forceDestroyItem : whether to destroy the items with ht->destroy
 */
void ht_destroy(Hashtable *ht, bool forceDestroyItem);

/*!
@relates Hashtable
@brief Prints the contents of the hashtable
@param[in] ht : the hashtable
 */

void ht_print(Hashtable ht);

#endif
