#include "spec_ids.h"
#include "lists.h"

int main(){
  StrList* result = get_spec_ids();
  print_list(result);
  llfree(result, (llfree_f)free_StrList_data);
}