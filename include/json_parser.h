#include "hash.h"
#include "lists.h"
#include <stdarg.h>
#include <stdlib.h>

typedef LISTOF(char*) StringList;

/*!
\defgroup json_parser Json Parser
This module provides a parser and a tokenizer for json inputs.

\brief JSON parser and tokenizer logic.

### Example usage
@code{.c}
void parse_obj(void) {

    char *rest_str;
    StringList *rest_toks;
    StringList *tokens = json_tokenize_str(
            "{\"foo\" : 5, \"bar\" : false, \"baz\" : [true]}", &rest_str);
    // rest_str should point to the null terminator of the input string
    // tokens should contain a list of all the tokens in the input string
    JSON_ENTITY *ent = json_parse_value(tokens, &rest_toks);
    // rest_toks should point to NULL
    // ent should be a JSON_OBJ with 3 keys

    JSON_ENTITY *foo, *bar, *baz;
    foo = json_get(ent, "\"foo\"");
    bar = json_get(ent, "\"bar\"");
    baz = json_get(ent, "\"baz\"");

    CLEANUP();
}
@endcode
@{
 */


/*!
\brief The possible types of a json value.
 */
typedef enum {
    /*! json object type */
    JSON_OBJ,
    /*! json array type */
    JSON_ARRAY,
    /*! json number type */
    JSON_NUM,
    /*! json boolean type */
    JSON_BOOL,
    /*! json string type */
    JSON_STRING,
    /*! json null type-like */
    JSON_NULL
} json_type;

/*!
\brief The representation used for json values.

When reasoning about it in code, the type should be consulted first and then
the appropriate access functions.
 */
typedef struct {
    /*! \brief the type of the value represented with this struct */
    const json_type type;
    /*! \brief the data associated with the value */
    char data[];
} JSON_ENTITY;

/*!
\brief Splits an input string to tokens.

Tokens produced from the tokenizer are:
+ "true"
+ "false"
+ "null"
+ "{"
+ "}"
+ ":"
+ "["
+ "]"
+ ","
+ numbers as defined in the json standard
+ strings as defined in the json standard


@param[in] input : the input string
@param[out] rest : a pointer to the postfix of str, that failed to be tokenized.
@returns a list of tokens
 */
StringList *json_tokenize_str(char *input, char **rest);

/*!
@brief Converts a json_type to a string representation.

The string is statically allocated and does not need to be freed
@param[in] type : the json_type
@returns a string that describes t
 */
char *json_type_to_str(json_type type);

/*!
@relates JSON_ENTITY
@brief Converts a JSON_ENTITY encapsulated double to a C double

@param[in] jsonEntity : A JSON_ENTITY with type JSON_NUM
@returns the number stored in the entity
 */
double json_to_double(JSON_ENTITY *jsonEntity);

/*!
@relates JSON_ENTITY
@brief Converts a JSON_ENTITY encapsulated string to a C string

@param[in] jsonEntity : A JSON_ENTITY with type JSON_STRING
@returns the string stored in the entity
 */
char *json_to_string(JSON_ENTITY *jsonEntity);

/*!
@relates JSON_ENTITY
@brief Converts a JSON_ENTITY encapsulated bool to a C bool

@param[in] jsonEntity : A JSON_ENTITY with type JSON_BOOL
@returns the bool stored in the entity
 */
bool json_to_bool(JSON_ENTITY *jsonEntity);

/*!
@relates JSON_ENTITY
@brief Converts a json string to JSON_ENTITY

@param[in] json : A json string
@returns the parsed JSON_ENTITY
 */
JSON_ENTITY *json_to_entity(char *json);

/*!
@relates JSON_ENTITY
@brief Get the length of a JSON_ARRAY

@param[in] jsonEntity : A JSON_ENTITY with type JSON_ARRAY
@returns the length of the array stored in the entity
 */
int json_get_arr_length(JSON_ENTITY *jsonEntity);

/*!
@relates JSON_ENTITY
@brief Get the keys of a JSON_OBJECT

@param[in] jsonEntity : A JSON_ENTITY with type JSON_OBJ
@returns the list of keys of the json object stored in the entity
 */
StringList *json_get_obj_keys(JSON_ENTITY *jsonEntity);

/*!
@relates JSON_ENTITY
@brief Get a member of the json value

can either be called as json_get(JSON_ENTITY*, int) or
json_get(JSON_ENTITY*, char*)
@param[in] jsonEntity : A JSON_ENTITY with type of either JSON_ARRAY or JSON_OBJ
@param[in] key_or_index : If Ent is a JSON_OBJ,
this is interpreted as a string and returns the JSON_ENTITY associated with the
key on the object. Otherwise, if Ent is a JSON_ARRAY, this is interpreted as an
int and the key_or_index-th element of the array is returned.
 */
JSON_ENTITY *json_get(JSON_ENTITY *jsonEntity, ...);

/*!
@relates JSON_ENTITY
@brief Frees a JSON_ENTITY. 

If it is an object or an array, this recursively frees the contents as well.
@param[in] jsonEntity : the entity to be freed
 */
void json_entity_free(JSON_ENTITY *jsonEntity);

/*!
@brief Parses a json value from a list of tokens.

The value can be of any of the types in the JSON standard
(ie. number, string, boolean, object, array).
The resulting JSON_ENTITY, if not NULL, should be freed with json_entity_free.
@param[in] tokens : A list of tokens
@param[out] rest : a pointer to the first unconsumed token, if all tokens were
consumed, NULL
@returns the JSON_ENTITY built by reading the tokens
 */
JSON_ENTITY *json_parse_value(StringList *tokens, StringList **rest);

/*!
@brief Prints a json value
@param[in] jsonEntity : the value to be printed
 */
void json_print_value(JSON_ENTITY *jsonEntity);

/*!
@brief Parses a list of tokens that represents a json file

@param[in] tokens : the list of tokens
 */
JSON_ENTITY *json_parse_from_tokens(StringList *tokens);

/*! @} */

/*!
@private
 */
void json_free_StringList(StringList *list);
