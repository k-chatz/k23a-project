#include "../include/json_parser.h"
#include <ctype.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

/*! @private */
typedef struct {
    int length;
    JSON_ENTITY **contents;
} JSON_ARRAY_DATA;

/*! @private */
typedef struct {
    StringList *keys;
    dictp contents;
} JSON_OBJECT_DATA;

static JSON_ENTITY *json_new_num(double num) {
    JSON_ENTITY *new = malloc(sizeof(*new) + sizeof(num));
    /* new->json_vt = json_int_vt; */
    *((json_type *) (&new->type)) = JSON_NUM; /* assign to const */
    *((double *) (&new->data)) = num;
    return new;
}

static JSON_ENTITY *json_new_str(char *str) {
    JSON_ENTITY *new = malloc(sizeof(*new) + sizeof(str));
    /* new->json_vt = json_int_vt; */
    *((json_type *) (&new->type)) = JSON_STRING; /* assign to const */
    *((char **) (&new->data)) = str;
    return new;
}

static JSON_ENTITY *json_new_bool(bool b) {
    JSON_ENTITY *new = malloc(sizeof(*new) + sizeof(b));
    /* new->json_vt = json_int_vt; */
    *((json_type *) (&new->type)) = JSON_BOOL; /* assign to const */
    *((bool *) (&new->data)) = b;
    return new;
}

static JSON_ENTITY *json_new_arr(JSON_ENTITY **jsonEntityArr, int length) {
    JSON_ENTITY *new = malloc(sizeof(*new) + sizeof(JSON_ARRAY_DATA));
    /* new->json_vt = json_int_vt; */
    *((json_type *) (&new->type)) = JSON_ARRAY; /* assign to const */
    JSON_ARRAY_DATA *datap = (void *) &new->data;
    datap->length = length;
    datap->contents = jsonEntityArr;
    return new;
}

static JSON_ENTITY *json_new_obj(hashp kvs, StringList *keys) {
    JSON_ENTITY *new = malloc(sizeof(*new) + sizeof(JSON_OBJECT_DATA));
    /* new->json_vt = json_int_vt; */
    *((json_type *) (&new->type)) = JSON_OBJ; /* assign to const */
    JSON_OBJECT_DATA *datap = (void *) &new->data;
    datap->keys = keys;
    datap->contents = (dictp) kvs;
    return new;
}

static JSON_ENTITY json_null_ent = {JSON_NULL};

char *json_type_to_str(json_type type) {
    switch (type) {
        case JSON_OBJ:
            return "JSON_OBJ";
        case JSON_ARRAY:
            return "JSON_ARRAY";
        case JSON_NUM:
            return "JSON_NUM";
        case JSON_BOOL:
            return "JSON_BOOL";
        case JSON_STRING:
            return "JSON_STRING";
        case JSON_NULL:
            return "JSON_NULL";
        default:
            return "JSON_INVALID";
    }
}

double json_to_double(JSON_ENTITY *jsonEntity) {
    if (jsonEntity->type != JSON_NUM) {
        fprintf(stderr,
                "Attempted to get double from json entity of type %s.\n",
                json_type_to_str(jsonEntity->type));
        return 0;
    }
    return *((double *) &(jsonEntity->data));
}

char *json_to_string(JSON_ENTITY *jsonEntity) {
    if (jsonEntity->type != JSON_STRING) {
        fprintf(stderr,
                "Attempted to get string from json entity of type %s.\n",
                json_type_to_str(jsonEntity->type));
        return 0;
    }
    return *((char **) &(jsonEntity->data));
}

bool json_to_bool(JSON_ENTITY *jsonEntity) {
    if (jsonEntity->type != JSON_BOOL) {
        fprintf(stderr, "Attempted to get bool from json entity of type %s.\n",
                json_type_to_str(jsonEntity->type));
        return 0;
    }
    return *((bool *) &(jsonEntity->data));
}

int json_get_arr_length(JSON_ENTITY *jsonEntity) {
    if (jsonEntity->type != JSON_ARRAY) {
        fprintf(stderr,
                "Attempted to get length from json entity of type %s(Expected "
                "JSON_ARRAY).\n",
                json_type_to_str(jsonEntity->type));
        return 0;
    }
    JSON_ARRAY_DATA *datap = (void *) (&jsonEntity->data);
    return datap->length;
}

StringList *json_get_obj_keys(JSON_ENTITY *jsonEntity) {
    if (jsonEntity->type != JSON_OBJ) {
        fprintf(stderr,
                "Attempted to get length from json entity of type %s(Expected "
                "JSON_ARRAY).\n",
                json_type_to_str(jsonEntity->type));
        return 0;
    }
    JSON_OBJECT_DATA *datap = (void *) (&jsonEntity->data);
    return datap->keys;
}

/*! @private */
struct JSON_OBJ_ENTRY {
    char *key;
    JSON_ENTITY *value;
};

JSON_ENTITY *json_get(JSON_ENTITY *jsonEntity, ...) {
    va_list arglist;
    va_start(arglist, jsonEntity);
    switch (jsonEntity->type) {
        case JSON_OBJ: {
            char *key = va_arg(arglist, char *);
            if (key == (void *) 0x1) {
                return NULL;
            }
            JSON_OBJECT_DATA *dp = (void *) (&jsonEntity->data);
            va_end(arglist);
            return *(JSON_ENTITY **) dict_get(dp->contents, key);
        }
        case JSON_ARRAY: {
            int index = va_arg(arglist, int);
            JSON_ARRAY_DATA *dp = (void *) (&jsonEntity->data);
            va_end(arglist);
            return dp->contents[index];
        }
        default:
            fprintf(stderr,
                    "Error in json_get. Expected either JSON_OBJ or "
                    "JSON_ARRAY. Got %s instead.",
                    json_type_to_str(jsonEntity->type));
            return NULL;
    }
}

void json_entityp_free(JSON_ENTITY **ent) { json_entity_free(*ent); }

void json_entity_free(JSON_ENTITY *jsonEntity) {
    if (jsonEntity == NULL)
        return;
    switch (jsonEntity->type) {
        case JSON_STRING:
            free(json_to_string(jsonEntity));
        case JSON_NUM:
        case JSON_BOOL:
            free(jsonEntity);
            break;
        case JSON_ARRAY: {
            int len = json_get_arr_length(jsonEntity);
            for (int i = 0; i < len; i++) {
                json_entity_free(json_get(jsonEntity, i));
            }
            free(((JSON_ARRAY_DATA *) (&jsonEntity->data))->contents);
            free(jsonEntity);
        }
            break;
        case JSON_OBJ: {
            dict_free(((JSON_OBJECT_DATA *) &jsonEntity->data)->contents,
                      (void (*)(void *)) json_entityp_free);
            ll_free(json_get_obj_keys(jsonEntity), (llfree_f) json_free_StringList);
            free(jsonEntity);
        }
            break;
        case JSON_NULL:
            break;
    }
}

/* static struct JSON_OBJ_ENTRY *new_json_obj_entry(char *key, JSON_ENTITY *val)
 * { */
/*     struct JSON_OBJ_ENTRY *new = malloc(sizeof(*new)); */
/*     new->key = key; */
/*     new->value = val; */
/*     return new; */
/* } */


void print_strlist(StringList *stringList) {
    for (StringList *x = stringList; x; x = ll_nth(x, 1)) {
        printf("%s\n", x->data);
    }
}

static JSON_ENTITY *json_parse_object(tokenizer_t *tok) {
    if (strcmp("{", tok->buf) == 0) {
        /* clang-format off */
        /*! @formatter:off */
        dictp kvs =
                dict_config
                        (dict_new2(16, sizeof(JSON_ENTITY *)),
                         DICT_CONF_HASH_FUNC, djb2_str,
                         DICT_CONF_KEY_SZ_F, str_sz,
                         DICT_CONF_CMP, (ht_cmp_func) strncmp,
                         DICT_CONF_KEY_CPY, (ht_key_cpy_func) strncpy,
                         DICT_CONF_DONE);
        /*! formatter:on */
        /* clang-format on */

        StringList *keys = NULL;
	char *token = tokenizer_next(tok);
        while (token[0] != '}') {
          char *key = strdup(token);
          StringList *keys_node = malloc(sizeof(StringList));
          keys_node->data = key;
          ll_push(&keys, keys_node);
          assert(tokenizer_next(tok)[0] == ':');
          JSON_ENTITY *value = json_parse_value(tok);
          dict_put(kvs, key, &value);

	  token = tokenizer_next(tok);
          assert((token[0] == ',' || token[0] == '}'));
	  if(token[0] == ',')
	      token = tokenizer_next(tok);
        }
        JSON_ENTITY *obj = json_new_obj((hashp) kvs, keys);
	return obj;
    }
    return NULL;
}

JSON_ENTITY *json_parse_array(tokenizer_t *tok) {
    if (strcmp(tok->buf, "[") == 0) {
	int arr_len = 0;
	int arr_cap = 16;
	JSON_ENTITY **arr_cont = malloc(sizeof(JSON_ENTITY *) * arr_cap);
	
        JSON_ENTITY *curr_ent = json_parse_value(tok);
        if (curr_ent) {
	    arr_cont[arr_len++] = curr_ent;
        }
        while (strcmp(tokenizer_next(tok), ",") == 0) {
            curr_ent = json_parse_value(tok);
            if (curr_ent) {
		if(arr_len + 1 > arr_cap) {
		    arr_cap = arr_cap << 2;
		    arr_cont = realloc(arr_cont, arr_cap * sizeof(JSON_ENTITY*));
		}
		arr_cont[arr_len++] = curr_ent;
            }
        }
	arr_cont = realloc(arr_cont, arr_len * sizeof(JSON_ENTITY*));
	JSON_ENTITY *arr = json_new_arr(arr_cont, arr_len);
	
        if (strcmp(tok->buf, "]") == 0) {
            /* we are done */
            return arr;
        }
	json_entity_free(arr);
    }
    return NULL;
}

JSON_ENTITY *json_parse_value(tokenizer_t *tok) {
    char *first_tok = tokenizer_next(tok);
    if (isdigit(first_tok[0]) || (first_tok[0] == '-' && isdigit(first_tok[1]))) {
        /* number */
        double num = atof(first_tok);
        return json_new_num(num);
    } else if (strcmp(first_tok, "true") == 0) {
        /* true */
        return json_new_bool(true);
    } else if (strcmp(first_tok, "false") == 0) {
        /* false */
        return json_new_bool(false);
    } else if (first_tok[0] == '"') {
        /* string */
        return json_new_str(strdup(first_tok));
    } else if (strcmp("{", first_tok) == 0) {
        /* object */
        return json_parse_object(tok);
    } else if (strcmp("[", first_tok) == 0) {
        /* array */
        return json_parse_array(tok);
    } else if (strcmp("null", first_tok) == 0) {
        /* null */
        return &json_null_ent;
    }
    return NULL;
}

JSON_ENTITY *json_parse_file(char *path) {
    tokenizer_t *tok = json_tokenizer_from_filename(path);
    JSON_ENTITY *out = json_parse_value(tok);
    tokenizer_free(tok);
    return out;
}

JSON_ENTITY *json_parse_string(char *str) {
    tokenizer_t *tok = json_tokenizer_from_string(str);
    JSON_ENTITY *out = json_parse_value(tok);
    tokenizer_free(tok);
    return out;
}

void json_print_value(JSON_ENTITY *jsonEntity) {
    if (jsonEntity == NULL) {
        return;
    }
    switch (jsonEntity->type) {
        case JSON_NUM:
            printf("%lf", json_to_double(jsonEntity));
            break;
        case JSON_NULL:
            printf("null");
            break;
        case JSON_BOOL:
            printf("%s", json_to_bool(jsonEntity) ? "true" : "false");
            break;
        case JSON_STRING:
            printf("%s", json_to_string(jsonEntity));
            break;
        case JSON_ARRAY: {
            int length = json_get_arr_length(jsonEntity);
            printf("[");
            json_print_value(json_get(jsonEntity, 0));
            for (int i = 1; i < length; i++) {
                printf(", ");
                json_print_value(json_get(jsonEntity, i));
            }
            printf("]");
        }
            break;
        case JSON_OBJ: {
            StringList *keys = json_get_obj_keys(jsonEntity);
            printf("{");
            LLFOREACH(key, keys) {
                printf("%s : ", key->data);
                json_print_value(json_get(jsonEntity, key->data));
            }
            printf("}");
        }
            break;
        default:
            printf("<JSON_INVALID@%p>", jsonEntity);
    }
}

void json_free_StringList(StringList *list) {
    free(list->data);
    free(list);
}
