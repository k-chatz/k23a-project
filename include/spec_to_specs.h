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

typedef LISTOF(char*) StrList;
typedef LISTOF(SpecEntry*) SpecList;

/*!
  @brief Data structure that relates a spec to other specs that refer to the same item
 */
typedef struct {
    /*! @brief Index of specs */
    Hashtable ht;
    /*! @brief List of specs, in case we need to iterate over them */
    StrList *keys;
} STS;

/*!
  @brief STS hashtable entry.
 */
struct SpecEntry_s {
    /*! @brief Spec id */
    char *id;
    /*! @brief Set of similar specs. */
    SpecList *similar;
};

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

void print_sts(STS *sts);

#endif
