#include "../include/acutest.h"
#include "../include/json_parser.h"
#include <stdbool.h>
#include <string.h>

#ifndef ACUTEST_H
#include <assert.h>

#define TEST_CHECK assert
#endif

#define ARR_LEN(ARR) (sizeof(ARR) / sizeof(ARR[0]))


/* ________________________________________ */
/* _____________ Parser tests ______________*/
/* ________________________________________ */

#define CLEANUP() json_entity_free(ent)

void parse_number(void) {
    JSON_ENTITY *ent = json_parse_string("5");
    TEST_CHECK(json_to_double(ent) == 5.0);
    CLEANUP();
}

void parse_bool(void) {
    JSON_ENTITY *ent = json_parse_string("true");
    TEST_CHECK(json_to_bool(ent) == true);
    CLEANUP();
}

void parse_array(void) {
    json_type arr_types[] = {JSON_BOOL, JSON_NUM, JSON_BOOL, JSON_NUM};
    JSON_ENTITY *ent = json_parse_string("[true, 5, false, -2.27]");
    TEST_CHECK(ent->type == JSON_ARRAY); /* type should be array */
    int len = json_get_arr_length(ent);
    TEST_CHECK(len == 4); /* length should be 4 */
    for (int i = 0; i < len; i++) {
        TEST_CHECK(json_get(ent, i)->type == arr_types[i]);
    }
    CLEANUP();
}

void parse_obj(void) {
    char *expected_keys[] = {"\"foo\"", "\"bar\"", "\"baz\""};
    JSON_ENTITY *ent = json_parse_string("{\"foo\" : 5, \"bar\" : false, \"baz\" : [true]}");
    TEST_CHECK(ent->type == JSON_OBJ); /* type should be array */
    StringList *keys = json_get_obj_keys(ent);
    TEST_CHECK(ll_len(keys) == 3); /* length should be 3 */
    bool removed = false;
    LLFOREACH(key, keys) {
        for (int i = 0; i < ARR_LEN(expected_keys); i++) {
            if (strcmp(key->data, expected_keys[i]) == 0) {
                expected_keys[i] = NULL;
                removed = true;
                break;
            }
        }
        TEST_CHECK(removed); /* check if key was in expected keys */
    }

    /* all expected keys should be empty now */
    TEST_CHECK(!(expected_keys[0] || expected_keys[1] || expected_keys[2]));

    JSON_ENTITY *foo, *bar, *baz;
    foo = json_get(ent, "\"foo\"");
    bar = json_get(ent, "\"bar\"");
    baz = json_get(ent, "\"baz\"");

    TEST_CHECK(foo->type == JSON_NUM);
    TEST_CHECK(bar->type == JSON_BOOL);
    TEST_CHECK(baz->type == JSON_ARRAY);
    TEST_CHECK(json_get(baz, 0)->type == JSON_BOOL);
    CLEANUP();
}

void parse_obj_complex(void) {

    char *json =
            "{"
            "    \"glossary\": {"
            "        \"title\": \"example glossary\","
            "		\"GlossDiv\": {"
            "            \"title\": \"S\","
            "			\"GlossList\": {"
            "                \"GlossEntry\": {"
            "                    \"ID\": \"SGML\","
            "					\"SortAs\": \"SGML\","
            "					\"GlossTerm\": \"Standard Generalized Markup Language\","
            "					\"Acronym\": \"SGML\","
            "					\"Abbrev\": \"ISO 8879:1986\","
            "					\"GlossDef\": {"
            "                        \"para\": \"A meta-markup language, used to create markup languages such as DocBook.\","
            "						\"GlossSeeAlso\": [\"GML\", \"XML\"]"
            "                    },"
            "					\"GlossSee\": \"markup\""
            "                }"
            "            }"
            "        }"
            "    }"
            "}";

    JSON_ENTITY *ent = json_parse_string(json);
    TEST_CHECK(ent != NULL);
    CLEANUP();
}


#ifndef ACUTEST_H

struct test_ {
    const char *name;

    void (*func)(void);
};

#define TEST_LIST const struct test_ test_list_[]
#endif

TEST_LIST = {{"parse_number",      parse_number},
             {"parse_bool",        parse_bool},
             {"parse_array",       parse_array},
             {"parse_obj",         parse_obj},
             {"parse_obj_complex", parse_obj_complex},
             {NULL, NULL}};

#ifndef ACUTEST_H

int main(int argc, char *argv[]) {
    int i;
    for (i = 0; test_list_[i].name != NULL; i++) {
        test_list_[i].func();
    }
    return 0;
}

#endif
