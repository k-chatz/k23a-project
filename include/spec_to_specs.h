/* file: spec_to_specs.h */
/* describes the data structure used to represent */
/* connections between specs */
#include <stdlib.h>
#include <string.h>

#include "hash.h"
#include "lists.h"

#define HT_CAP 128
#define HT_BSZ 256

typedef struct SpecEntry_s SpecEntry;

typedef LISTOF(char*) StrList;
typedef LISTOF(SpecEntry*) SpecList;

typedef
struct {
    Hashtable ht;
    StrList *keys;
} STS;


struct SpecEntry_s {
    char *id;
    SpecList *similar;
};


STS *sts_new();

int sts_add(STS *sts, char *id);

int sts_merge(STS *sts, char *id1, char *id2);

SpecEntry *sts_get(STS *sts, char *id);

void print_sts(STS *sts);

