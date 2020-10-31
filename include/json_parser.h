#include "hash.h"
#include "lists.h"
#include <stdarg.h>
#include "spec_ids.h" /* for StrList */
#include <stdlib.h>

typedef enum {
    JSON_OBJ,
    JSON_ARRAY,
    JSON_NUM,
    JSON_BOOL,
    JSON_STRING
} json_type;

struct json_entity_vt {
    char *(*toStr)(void *this);
};

typedef struct {
    /* struct json_entity_vt *json_vt; */
    const json_type type;
    char data[];
} JSON_ENTITY;

StrList *json_tokenize_str(char *str, char **rest);

struct JSON_OBJ_ENTRY {
    char *key;
    JSON_ENTITY *value;
};

typedef struct {
    int length;
    JSON_ENTITY **contents;
} JSON_ARRAY_DATA;

typedef struct {
    StrList *keys;
    Hashtable contents;
} JSON_OBJECT_DATA;

double json_to_double(JSON_ENTITY *Ent);

char *json_to_string(JSON_ENTITY *Ent);

bool json_to_bool(JSON_ENTITY *Ent);

int json_get_arr_length(JSON_ENTITY *Ent);

StrList *json_get_obj_keys(JSON_ENTITY *Ent);

JSON_ENTITY *json_get(JSON_ENTITY *Ent, ...);

void json_entity_free(JSON_ENTITY *ent);

JSON_ENTITY *json_parse_value(StrList *tokens, StrList **rest);

JSON_ENTITY *json_new_num(double num);

JSON_ENTITY *json_new_str(char *str);

JSON_ENTITY *json_new_bool(bool b);

JSON_ENTITY *json_new_arr(JSON_ENTITY **arr, int length);

JSON_ENTITY *json_new_obj(Hashtable kvs, StrList *keys);

char *json_type_to_str(json_type t);

struct JSON_OBJ_ENTRY *new_json_obj_entry(char *key, JSON_ENTITY *val);

ulong json_obj_entry_free(void *joe);

struct JSON_OBJ_ENTRY *json_parse_object_entry(StrList *tokens, StrList **rest);

JSON_ENTITY *json_parse_object(StrList *tokens, StrList **rest);

JSON_ENTITY *json_parse_array(StrList *tokens, StrList **rest);

JSON_ENTITY *json_parse_value(StrList *tokens, StrList **rest);

void json_print_value(JSON_ENTITY *val);

JSON_ENTITY *json_parse_from_tokens(StrList *tokens);
