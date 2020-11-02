#include "../include/json_parser.h"
#include <ctype.h>
#include <stdlib.h>
#include <string.h>

#define ARR_LEN(ARR) (sizeof(ARR) / sizeof(ARR[0]))
#define DIGITS "0123456789"

typedef char *(*matcher)(char *input);

#define MATCH_TOKEN(FNAME, TOK)                                                \
    static char *FNAME(char *input) {                                          \
        int cmp = strncmp(input, TOK, strlen(TOK));                            \
        if (!cmp)                                                              \
            return strdup(TOK);                                                \
        return strdup("");                                                     \
    }

MATCH_TOKEN(is_true, "true");

MATCH_TOKEN(is_false, "false");

MATCH_TOKEN(is_null, "null");

MATCH_TOKEN(is_lbrace, "{");

MATCH_TOKEN(is_rbrace, "}");

MATCH_TOKEN(is_comma, ",");

MATCH_TOKEN(is_colon, ":");

MATCH_TOKEN(is_lbracket, "[");

MATCH_TOKEN(is_rbracket, "]");

static char *is_number(char *input) {
    enum { INTEGER, FRAC, EXP, DONE } state = INTEGER;
    size_t total_size = 0;
    while (state != DONE) {
        switch (state) {
        case INTEGER:
            if (input[total_size] == '-')
                total_size++;
            if (input[total_size] == '0') {
                total_size++;
            } else if (input[total_size] > '0' && input[total_size] <= '9') {
                total_size++;
                total_size += strspn(input + total_size, DIGITS);
            } else {
                state = DONE;
            }
            state = FRAC;
            break;
        case FRAC:
            if (input[total_size] == '.') {
                size_t frac_size = 1;
                frac_size += strspn(input + total_size + 1, DIGITS);
                if (frac_size > 1) {
                    total_size += frac_size;
                }
            }
            state = EXP;
            break;
        case EXP:
            if (strspn(input + total_size, "eE") == 1) {
                size_t exp_size = 0;
                size_t digits_size = 0;
                exp_size++;
                if (strspn(input + total_size + exp_size, "+-") == 1)
                    exp_size++;
                digits_size = strspn(input + total_size + exp_size, DIGITS);
                if (digits_size > 0)
                    total_size += exp_size + digits_size;
            }
            state = DONE;
            break;
        case DONE:
            break;
        }
    }

    char *num = strndup(input, total_size);
    return num;
}

static char *is_string(char *input) {
    int curr = 1;
    if (input[0] == '"') {
        while (input[curr] != '"') {
            if (input[curr] == '\\') {
                /* escape */
                switch (input[curr + 1]) {
                case 'u':
                    curr += 4;
                case '"':
                case '\\':
                case '/':
                case 'b':
                case 'f':
                case 'n':
                case 'r':
                case 't':
                    curr += 2;
                    break;
                }
            } else {
                curr++;
            }
        }
        if (input[curr] == '"') {
            char *out = strndup(input, curr + 1);
            return out;
        }
    }
    return strdup("");
}

static char *json_next_token(char *input, int *cursor) {
    matcher tokenizers[] = {is_true,     is_false,  is_null,   is_lbrace,
                            is_rbrace,   is_comma,  is_colon,  is_lbracket,
                            is_rbracket, is_number, is_string, NULL};

    char *match;
    for (int i = 0; tokenizers[i] != NULL; i++) {
        match = tokenizers[i](input);
        int len = strlen(match);
        if (strlen(match) > 0) {
            *cursor = len;
            return match;
        } else {
            free(match);
        }
    }
    *cursor = 0;
    return strdup("");
}

static char *eat_ws(char *str) {
    char *out = str;
    while (isspace(*out))
        out++;
    return out;
}

StrList *json_tokenize_str(char *str, char **rest) {
    StrList *out = NULL;
    char *current;
    int current_len;
    do {
        str = eat_ws(str);
        current = json_next_token(str, &current_len);
        str += current_len;
        if (current_len > 0) {
            StrList *thisTok = malloc(sizeof(StrList));
            thisTok->data = current;
            ll_push(&out, thisTok);
        } else {
            free(current);
        }
    } while (str[0] != '\0' && current_len > 0);

    ll_reverse(&out);
    *rest = str;
    return out;
}

/* ---------- JSON Parser start ---------- */

/*! @private */
typedef struct {
    int length;
    JSON_ENTITY **contents;
} JSON_ARRAY_DATA;

/*! @private */
typedef struct {
    StrList *keys;
    Hashtable contents;
} JSON_OBJECT_DATA;

static JSON_ENTITY *json_new_num(double num) {
    JSON_ENTITY *new = malloc(sizeof(*new) + sizeof(num));
    /* new->json_vt = json_int_vt; */
    *((json_type *)(&new->type)) = JSON_NUM; /* assign to const */
    *((double *)(&new->data)) = num;
    return new;
}

static JSON_ENTITY *json_new_str(char *str) {
    JSON_ENTITY *new = malloc(sizeof(*new) + sizeof(str));
    /* new->json_vt = json_int_vt; */
    *((json_type *)(&new->type)) = JSON_STRING; /* assign to const */
    *((char **)(&new->data)) = str;
    return new;
}

static JSON_ENTITY *json_new_bool(bool b) {
    JSON_ENTITY *new = malloc(sizeof(*new) + sizeof(b));
    /* new->json_vt = json_int_vt; */
    *((json_type *)(&new->type)) = JSON_BOOL; /* assign to const */
    *((bool *)(&new->data)) = b;
    return new;
}

static JSON_ENTITY *json_new_arr(JSON_ENTITY **arr, int length) {
    JSON_ENTITY *new = malloc(sizeof(*new) + sizeof(JSON_ARRAY_DATA));
    /* new->json_vt = json_int_vt; */
    *((json_type *)(&new->type)) = JSON_ARRAY; /* assign to const */
    JSON_ARRAY_DATA *datap = (void *)&new->data;
    datap->length = length;
    datap->contents = arr;
    return new;
}

static JSON_ENTITY *json_new_obj(Hashtable kvs, StrList *keys) {
    JSON_ENTITY *new = malloc(sizeof(*new) + sizeof(JSON_OBJECT_DATA));
    /* new->json_vt = json_int_vt; */
    *((json_type *)(&new->type)) = JSON_OBJ; /* assign to const */
    JSON_OBJECT_DATA *datap = (void *)&new->data;
    datap->keys = keys;
    datap->contents = kvs;
    return new;
}

static JSON_ENTITY json_null_ent = {JSON_NULL};

char *json_type_to_str(json_type t) {
    switch (t) {
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

double json_to_double(JSON_ENTITY *Ent) {
    if (Ent->type != JSON_NUM) {
        fprintf(stderr,
                "Attempted to get double from json entity of type %s.\n",
                json_type_to_str(Ent->type));
        return 0;
    }
    return *((double *)&(Ent->data));
}

char *json_to_string(JSON_ENTITY *Ent) {
    if (Ent->type != JSON_STRING) {
        fprintf(stderr,
                "Attempted to get string from json entity of type %s.\n",
                json_type_to_str(Ent->type));
        return 0;
    }
    return *((char **)&(Ent->data));
}

bool json_to_bool(JSON_ENTITY *Ent) {
    if (Ent->type != JSON_BOOL) {
        fprintf(stderr, "Attempted to get bool from json entity of type %s.\n",
                json_type_to_str(Ent->type));
        return 0;
    }
    return *((bool *)&(Ent->data));
}

int json_get_arr_length(JSON_ENTITY *Ent) {
    if (Ent->type != JSON_ARRAY) {
        fprintf(stderr,
                "Attempted to get length from json entity of type %s(Expected "
                "JSON_ARRAY).\n",
                json_type_to_str(Ent->type));
        return 0;
    }
    JSON_ARRAY_DATA *datap = (void *)(&Ent->data);
    return datap->length;
}

StrList *json_get_obj_keys(JSON_ENTITY *Ent) {
    if (Ent->type != JSON_OBJ) {
        fprintf(stderr,
                "Attempted to get length from json entity of type %s(Expected "
                "JSON_ARRAY).\n",
                json_type_to_str(Ent->type));
        return 0;
    }
    JSON_OBJECT_DATA *datap = (void *)(&Ent->data);
    return datap->keys;
}

/*! @private */
struct JSON_OBJ_ENTRY {
    char *key;
    JSON_ENTITY *value;
};

JSON_ENTITY *json_get(JSON_ENTITY *Ent, ...) {
    va_list arglist;
    va_start(arglist, Ent);
    switch (Ent->type) {
    case JSON_OBJ: {
        char *key = va_arg(arglist, char *);
        JSON_OBJECT_DATA *dp = (void *)(&Ent->data);
        va_end(arglist);
        return ((struct JSON_OBJ_ENTRY *)ht_get(dp->contents, key))->value;
    }
    case JSON_ARRAY: {
        int index = va_arg(arglist, int);
        JSON_ARRAY_DATA *dp = (void *)(&Ent->data);
        va_end(arglist);
        return dp->contents[index];
    }
    default:
        fprintf(stderr,
                "Error in json_get. Expected either JSON_OBJ or "
                "JSON_ARRAY. Got %s instead.",
                json_type_to_str(Ent->type));
        return NULL;
    }
}

void json_entity_free(JSON_ENTITY *ent) {
    switch (ent->type) {
    case JSON_STRING:
        free(json_to_string(ent));
    case JSON_NUM:
    case JSON_BOOL:
        free(ent);
        break;
    case JSON_ARRAY: {
        int len = json_get_arr_length(ent);
        for (int i = 0; i < len; i++) {
            json_entity_free(json_get(ent, i));
        }
        free(((JSON_ARRAY_DATA *)(&ent->data))->contents);
        free(ent);
    } break;
    case JSON_OBJ: {
        ht_destroy(&((JSON_OBJECT_DATA *)&ent->data)->contents, true);
        ll_free(json_get_obj_keys(ent), free);
        free(ent);
    } break;
    case JSON_NULL:
        break;
    }
}

/* static struct JSON_OBJ_ENTRY *new_json_obj_entry(char *key, JSON_ENTITY *val) { */
/*     struct JSON_OBJ_ENTRY *new = malloc(sizeof(*new)); */
/*     new->key = key; */
/*     new->value = val; */
/*     return new; */
/* } */

static void *ht_create_id(void *valargs) { return valargs; }

static int json_obj_entry_cmp(void *obj, void *key) {
    return strcmp(((struct JSON_OBJ_ENTRY *)obj)->key, key);
}

static ulong hash_str(void *id, ulong htcap) {
    ulong sum = 0;
    while (*(char *)id) {
        sum *= 47; /* multiply by a prime number */
        sum += *((char *)id);
        id++;
    }
    return sum % htcap;
}

static ulong json_obj_entry_free(void *joe) {
    struct JSON_OBJ_ENTRY *e = joe;
    free(e->key);
    json_entity_free(e->value);
    free(joe);
    return 0;
}

static struct JSON_OBJ_ENTRY *json_parse_object_entry(StrList *tokens,
                                               StrList **rest) {
    StrList *key, *colon, *val_start;
    key = ll_nth(tokens, 0);
    colon = ll_nth(tokens, 1);
    val_start = ll_nth(tokens, 2);

    if (!val_start)
        return NULL;

    if (key->data[0] == '"' && strcmp(colon->data, ":") == 0) {
        JSON_ENTITY *val = json_parse_value(val_start, rest);
        if (val) {
            struct JSON_OBJ_ENTRY *entry = malloc(sizeof(*entry));
            entry->key = strdup(key->data);
            entry->value = val;
            return entry;
        }
    }
    *rest = tokens;
    return NULL;
}

static JSON_ENTITY *json_parse_object(StrList *tokens, StrList **rest) {
    *rest = tokens;
    if (strcmp("{", (*rest)->data) == 0) {
        *rest = ll_nth(*rest, 1);
        Hashtable kvs;
        ht_init(&kvs, 10, 2 * sizeof(void *) + sizeof(ulong), &ht_create_id,
                &json_obj_entry_cmp, NULL, &hash_str, json_obj_entry_free);
        StrList *keys = NULL;
        struct JSON_OBJ_ENTRY *new_ent = json_parse_object_entry(*rest, rest);
        while (new_ent) {
            StrList *keys_node = malloc(sizeof(StrList));
            keys_node->data = new_ent->key;
            ll_push(&keys, keys_node);
            void *_;
            ht_insert(kvs, new_ent->key, new_ent, &_);
            if (strcmp((*rest)->data, ",") == 0)
                new_ent = json_parse_object_entry(ll_nth(*rest, 1), rest);
            else
                new_ent = NULL;
        }
        JSON_ENTITY *obj = json_new_obj(kvs, keys);
        if (strcmp("}", (*rest)->data) == 0) {
            *rest = ll_nth(*rest, 1);
            return obj;
        }
        json_entity_free(obj);
    }
    return NULL;
}

static JSON_ENTITY *json_parse_array(StrList *tokens, StrList **rest) {
    if (strcmp(tokens->data, "[") == 0) {
        struct tmplist {
            struct tmplist *next;
            JSON_ENTITY *ent;
        } *data = NULL;
        *rest = ll_nth(tokens, 1); /* skip '[' */
        JSON_ENTITY *curr_ent = json_parse_value(*rest, rest);
        if (curr_ent) {
            void *newnode = malloc(sizeof(*data));
            ll_push(&data, newnode);
            data->ent = curr_ent;
        }
        while (strcmp((*rest)->data, ",") == 0) {
            curr_ent = json_parse_value(ll_nth(*rest, 1), rest);
            if (curr_ent) {
                void *newnode = malloc(sizeof(*data));
                ll_push(&data, newnode);
                data->ent = curr_ent;
            }
        }
        /* make the array */
        int length = ll_len(data);
        JSON_ENTITY **arr = malloc(sizeof(JSON_ENTITY *) * length);
        int i = length;
        LLFOREACH(ent, data) { arr[--i] = ent->ent; }
        ll_free(data, free); /* free the temp list */
        if (strcmp((*rest)->data, "]") == 0) {
            /* we are done */
            *rest = ll_nth(*rest, 1);
            return json_new_arr(arr, length);
        }

        for (i = 0; i < length; i++) {
            json_entity_free(arr[i]);
        }
        free(arr);
    }
    *rest = tokens; /* consume no tokens */
    return NULL;
}

JSON_ENTITY *json_parse_value(StrList *tokens, StrList **rest) {
    char *first_tok = tokens->data;
    if ((first_tok[0] >= '0' && first_tok[0] <= '9') ||
        (first_tok[0] == '-' && first_tok[1] >= '0' && first_tok[1] <= '9')) {
        /* number */
        double num = atof(first_tok);
        *rest = ll_nth(tokens, 1);
        return json_new_num(num);
    } else if (strcmp(first_tok, "true") == 0) {
        /* true */
        *rest = ll_nth(tokens, 1);
        return json_new_bool(true);
    } else if (strcmp(first_tok, "false") == 0) {
        /* false */
        *rest = ll_nth(tokens, 1);
        return json_new_bool(false);
    } else if (first_tok[0] == '"') {
        /* string */
        *rest = ll_nth(tokens, 1);
        return json_new_str(strdup(first_tok));
    } else if (strcmp("{", first_tok) == 0) {
        /* object */
        return json_parse_object(tokens, rest);
    } else if (strcmp("[", first_tok) == 0) {
        /* array */
        return json_parse_array(tokens, rest);
    } else if(strcmp("null", first_tok) == 0){
	/* null */
	return &json_null_ent;
    }
    *rest = tokens;
    return NULL;
}

void json_print_value(JSON_ENTITY *val) {
    switch (val->type) {
    case JSON_NUM:
        printf("%lf", json_to_double(val));
        break;
    case JSON_NULL:
	printf("null");
	break;
    case JSON_BOOL:
        printf("%s", json_to_bool(val) ? "true" : "false");
        break;
    case JSON_STRING:
        printf("%s", json_to_string(val));
        break;
    case JSON_ARRAY: {
        int length = json_get_arr_length(val);
        printf("[");
        json_print_value(json_get(val, 0));
        for (int i = 1; i < length; i++) {
            printf(", ");
            json_print_value(json_get(val, i));
        }
        printf("]");
    } break;
    case JSON_OBJ: {
        StrList *keys = json_get_obj_keys(val);
        printf("{");
        LLFOREACH(key, keys) {
            printf("%s : ", key->data);
            json_print_value(json_get(val, key->data));
        }
        printf("}");
    } break;
    default:
        printf("<JSON_INVALID@%p>", val);
    }
}

void json_free_StrList(StrList *list) {
    free(list->data);
    free(list);
}
