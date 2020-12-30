/* file: spec_to_specs.h */
/* describes the data structure used to represent */
/* connections between specs */

#ifndef __STS_H__
#define __STS_H__

#include <stdlib.h>
#include <string.h>

#include "hash.h"
#include "lists.h"

#define HT_CAP 128
#define HT_BSZ 256

#define MATCHES_CHUNK_SIZE 1000

typedef struct SpecEntry_s SpecEntry;

typedef LISTOF(char *) StrList;
typedef LISTOF(SpecEntry *) SpecList;

/*!
  @brief Data structure that relates a spec to other specs that refer to the
  same item
 */
typedef struct {
    /*! @brief Index of specs */
    dictp dict;
} STS;

enum pair_type {
    NAT,
    TRAIN,
    TEST,
    VALIDATION
};

typedef struct pair {
    char *spec1;
    char *spec2;
    int relation;
    enum pair_type type;
} Pair;

/*!
  @brief STS hashtable entry.
 */
struct SpecEntry_s {
    /*! @brief spec id */
    char *id;
    /*! @brief Set of similar specs. */
    char *parent;
    /*! @brief Contents of the set
      if this node is the representative of the set, this is the list of the elements;
      otherwise, this is NULL
     */
    StrList *similar, *similar_tail, *different, *different_tail;
    /*! @brief Length of similar */
    ulong similar_len, different_len;

    bool printed;
};

SpecEntry *findRoot(STS *sts, SpecEntry *spec);

/*!
  @brief Creates a new STS structure
*/
STS *sts_new();

/*!
  @relates STS

  @brief Destroys an STS structure
  @param[in] sts : the sts structure
*/
void sts_destroy(STS *sts);

/*!
  @relates STS

  @brief Adds a new entry to the STS
  @param[in] sts : the sts structure
  @param[in] id : the id to be added
  @returns void
*/
int sts_add(STS *sts, char *id);

/*!
  @relates STS

  @brief Marks two spec ids as similar, unifying their similar sets.
  @param[in] sts : the sts structure
  @param[in] id1 : first id to be merged
  @param[in] id2 : second id to be merged

 */
int sts_merge(STS *sts, char *id1, char *id2);

/*!
  @relates STS

  @brief Marks two spec ids as similar, unifying their similar sets.
  @param[in] sts : the sts structure
  @param[in] id1 : first id to be checked for differences
  @param[in] id2 : second id to be checked for differences

 */
int sts_diff(STS *sts, char *id1, char *id2);

/*!
  @relates STS

  @brief Gets the SpecEntry of a specific id.

  @param[in] sts : the sts structure
  @param[in] id : the id of the entry that will be returned

  @returns SpecEntry of a given id.
*/
SpecEntry *sts_get(STS *sts, char *id);

/*!
  @relates STS

  @brief Used to print the dot structure

  @param[in] file : the output file
  @param[in] sts : the sts structure
  @param[in] verbose : if True prints specs thoroughly, else prints only similar

  @returns void
*/

void print_sts_dot(FILE *file, STS *sts, bool verbose);

/*!
  @relates STS

  @brief Prints the the similar pairs

  @param[in] file : the output file
  @param[in] sts : the sts structure

  @returns void
*/
void init_similar_pairs(FILE *file, STS *sts, Pair **pairs, int *chunks, int *counter);

void init_different_pairs(FILE *file, STS *sts, Pair **pairs, int *chunks, int *counter);

void print_sts_similar(FILE *file, STS *sts);

void print_sts_diff(FILE *file, STS *sts);

#endif
