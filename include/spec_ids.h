#ifndef __PARSER_H__
#define __PARSER_H__

#include "lists.h"

typedef LISTOF(char*) StrList;

StrList *create_node(char *spec);

void print_list(StrList *list);

void free_StrList_data(StrList *list);

StrList *get_spec_ids();

#endif