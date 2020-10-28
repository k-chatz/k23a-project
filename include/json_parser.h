#include "lists.h"
#include "hash.h"

#include "spec_ids.h"		/* for StrList */

typedef Hashtable JSON_ENTITY;

StrList *json_tokenize_str(char *str);
JSON_ENTITY json_parse_from_tokens(StrList *tokens);
