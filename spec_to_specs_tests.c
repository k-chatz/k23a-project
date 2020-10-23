#include "spec_to_specs.h"

#define N (sizeof(ids) / sizeof(ids[0]))

void print_sts(STS *sts){
  StrList *keys = sts->keys;
  while(keys){
    SpecEntry *sp = HT_Get(sts->ht, keys->data);
    printf("%s -> (", sp->id);
    SpecList *similar = sp->similar;
    while(similar){
      printf("%s ", similar->data->id);
      similar = llnth(similar, 1);
    }
    printf(")\n");
    keys = llnth(keys, 1);
  }
}

int main(int argc, char *argv[]) {
  STS *sts = sts_new();
  char *ids[] = {"1", "2", "3", "4", "5"};
  int i;
  for(i = 0; i < N; i++){
    sts_add(sts, ids[i]);
  }
  print_sts(sts);

  sts_merge(sts, "1", "4");

  print_sts(sts);

  sts_merge(sts, "4", "5");

  print_sts(sts);
  
  return 0;
}
