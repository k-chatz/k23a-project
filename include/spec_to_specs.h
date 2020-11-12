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

typedef struct SpecEntry_s SpecEntry;

typedef LISTOF(char *) StrList;
typedef LISTOF(SpecEntry *) SpecList;

/*!
  @brief Data structure that relates a spec to other specs that refer to the
  same item
 */
typedef struct {
    /*! @brief Index of specs */
    hashp ht;
} STS;

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
    StrList *similar, *similar_tail;
    /*! @brief Length of similar */
    ulong similar_len;
};

SpecEntry *findRoot(STS *sts, SpecEntry *spec);

/*!
  @brief Creates a new STS structure
*/
STS *sts_new();

/*!
  @relates STS

  @brief Adds a new entry to the STS
  @param[in] sts : the sts structure
  @param[in] id : the id to be added
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

SpecEntry *sts_get(STS *sts, char *id);

void print_sts(FILE *file, STS *sts, bool verbose);

#endif
