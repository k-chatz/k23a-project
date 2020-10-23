/* file: spec_to_specs.h */

#include "hash.h"
#include "lists.h"
#include <stdlib.h>
#include <string.h>


#define HT_CAP 128
#define HT_BSZ 16

typedef LISTOF(SpecEntry) SpecList;

typedef
struct {
  char *id;
  SpecList *similar;
} SpecEntry;

void *mk_spec(void *id){
  SpecEntry *new = malloc(sizeof(SpecEntry));
  new->id = strdup(id);
  new->similar = malloc(sizeof(SpecList));
  new->similar->next = NULL;
  new->similar->data = new;	/* add self to list of similar specs */
  return new;
}

ulong hash_spec(void *spec, ulong htcap){
  ulong sum = 0;
  char *id = ((SpecEntry*)spec)->id;
  while(id){
    sum *= 47;			/* multiply by a prime number */
    sum += *id;
    id++;
  }
  return sum % htcap;
}

int cmp_spec(void *spec, void *key){
  return strcmp(((SpecEntry*)spec)->id, key);
}

ulong destroy_spec(void *spec){
  SpecList **similarp = &(((SpecEntry *)spec)->similar);
  while(*similarp){
    if(strcmp(((SpecEntry*)spec)->id, (*similarp)->data->id) == 0){
      /* remove this element */
      SpecList *poped = llpop(similarp);
      free(poped);
    }
    similarp = llnth(similarp, 1);
  }
  free(((SpecEntry*)spec)->id);
  free(spec);
  return 1;
}

Hashtable sts_init(){
  Hashtable new;
  HT_Init(&new,
	  HT_CAP,
	  HT_BSZ,
	  mk_spec,
	  cmp_spec,
	  hash_spec,
	  destroy_spec);
  return new;
}

void sts_add(char *id){
  HT_Insert(ht, id, void *valueParams, void **value)
}


