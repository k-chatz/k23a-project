#include "../include/acutest.h"
#include "../include/json_parser.h"
#include <stdbool.h>
#include <string.h>

#ifndef ACUTEST_H
#include <assert.h>

#define TEST_CHECK assert
#endif

#define ARR_LEN(ARR) (sizeof(ARR) / sizeof(ARR[0]))

bool tokenize_word(char *word) {
    StringList *tokens;
    char *rest;
    tokens = json_tokenize_str(word, &rest);
    bool success = (ll_len(tokens) == 1) && (strcmp(tokens->data, word) == 0);
    ll_free(tokens, (llfree_f) json_free_StringList);
    return success;
}

bool tokenize_sentence(char *str, char **expected_tokens) {
    StringList *tokens;
    char *rest;
    tokens = json_tokenize_str(str, &rest);
    int i = 0;
    for (StringList *tok = tokens; tok; tok = ll_nth(tok, 1)) {
        if (expected_tokens[i] == NULL)
            return false;
        if (strcmp(tok->data, expected_tokens[i++]))
            return false;
    }
    ll_free(tokens, (llfree_f) json_free_StringList);
    return expected_tokens[i] == NULL;
}

void tokenize_true(void) { TEST_CHECK(tokenize_word("true")); }

void tokenize_false(void) { TEST_CHECK(tokenize_word("false")); }

void tokenize_null(void) { TEST_CHECK(tokenize_word("null")); }

void tokenize_lbrace(void) { TEST_CHECK(tokenize_word("{")); }

void tokenize_rbrace(void) { TEST_CHECK(tokenize_word("}")); }

void tokenize_colon(void) { TEST_CHECK(tokenize_word(":")); }

void tokenize_lbracket(void) { TEST_CHECK(tokenize_word("[")); }

void tokenize_rbracket(void) { TEST_CHECK(tokenize_word("]")); }

void tokenize_comma(void) { TEST_CHECK(tokenize_word(",")); }

void tokenize_number(void) {
    typedef struct {
        char *str;
        bool valid;
    } str_valid;

    str_valid digits[] = {{"0",       true},
                          {"000",     true},
                          {"123",     true},
                          {"500",     true},
                          {"1238904", true}};

    str_valid ints[] = {
            {"0",      true},
            {"1%1$s",  true},
            {"2%1$s",  true},
            {"3%1$s",  true},
            {"4%1$s",  true},
            {"5%1$s",  true},
            {"6%1$s",  true},
            {"7%1$s",  true},
            {"8%1$s",  true},
            {"9%1$s",  true},
            {"-0",     true},
            {"-1%1$s", true},
            {"-2%1$s", true},
            {"-3%1$s", true},
            {"-4%1$s", true},
            {"-5%1$s", true},
            {"-6%1$s", true},
            {"-7%1$s", true},
            {"-8%1$s", true},
            {"-9%1$s", true},
            {"01",     false},
            {"-01",    false},
            {"--5",    false}};

    str_valid frac[] = {{"",      true},
                        {".%2$s", true},
                        {".",     false},
                        {".5.1",  false},
                        {"..",    false}};

    str_valid exp[] = {
            {"",       true},
            {"E%3$s",  true},
            {"E+%3$s", true},
            {"E-%3$s", true},
            {"e%3$s",  true},
            {"e+%3$s", true},
            {"e-%3$s", true},
            {"e",      false},
            {"e+",     false},
            {"e-",     false},
            {"E",      false},
            {"E+",     false},
            {"E-",     false},
    };

    char buf1[50], buf2[50];
    for (int i = 0; i < ARR_LEN(ints); i++) {
        str_valid integer_part = ints[i];
        bool expected_success;
        for (int j = 0; j < ARR_LEN(frac); j++) {
            str_valid frac_part = frac[j];
            for (int k = 0; k < ARR_LEN(exp); k++) {
                str_valid exp_part = exp[k];
                expected_success =
                        integer_part.valid && frac_part.valid && exp_part.valid;
                strcpy(buf1, integer_part.str);
                strcat(buf1, frac_part.str);
                strcat(buf1, exp_part.str);
                for (int d1 = 0; d1 < ARR_LEN(digits); d1++) {
                    for (int d2 = 0; d2 < ARR_LEN(digits); d2++) {
                        for (int d3 = 0; d3 < ARR_LEN(digits); d3++) {
                            sprintf(buf2, buf1, digits[1].str, digits[2].str,
                                    digits[3].str);
                            bool success = tokenize_word(buf2);
                            TEST_CHECK(success == expected_success);
                        }
                    }
                }
            }
        }
    }
}

void tokenize_string(void) {
    char *str_contents[] = {"fhasdj", "foo bar", "foo 123bar _@@!@#$%^&*()_+/",
      "\\\"", "\\uAbFf"};
    char buf[100];
    for (int i = 0; i < ARR_LEN(str_contents); i++) {
        sprintf(buf, "\"%s\"", str_contents[i]);
        TEST_CHECK(tokenize_word(buf));
    }
}

void tokenize_big_string(void) {
    char *bigstr = "\"Used: An item that has been used previously. The item may have some signs of cosmetic wear, but is fully\noperational and functions as intended. This item may be a floor model or store return that has been used. See the seller\\u00e2\\u0080\\u0099s listing for full details and description of any imperfections.\nSee all condition definitions- opens in a new window or tab\n... Read moreabout the condition\"";
    
    TEST_CHECK(tokenize_word(bigstr));
}


void tokenize_whitespace(void) {
    /* tokenizing whitespace should yield no tokens and consume it */
    char *rest;
    StringList *toks = json_tokenize_str("  \t \n ", &rest);
    TEST_CHECK(!toks);
    TEST_CHECK(strlen(rest) == 0);
}

void tokenize_multiword(void) {
    char *in_toks[] = {"5e2", "true", "false"};
    char *in_str = "5e2true   \t\n\n false";
    char *out_toks[ARR_LEN(in_toks)];
    char *rest;
    StringList *toks = json_tokenize_str(in_str, &rest);
    int i = 0;
    LLFOREACH(tok, toks) { out_toks[i++] = tok->data; }
    for (i = 0; i < ARR_LEN(in_toks); i++) {
        TEST_CHECK(strcmp(in_toks[i], out_toks[i]) == 0);
    }

    /* check if we consumed all the input */
    TEST_CHECK(strlen(rest) == 0);
    ll_free(toks, (llfree_f) json_free_StringList);
}

void tokenize_invalid(void) {
    char *rest;
    char *invalid_in = "treu";
    TEST_CHECK(json_tokenize_str(invalid_in, &rest) == NULL);
    /* no input should be consumed here */
    TEST_CHECK(strcmp(rest, invalid_in) == 0);
}

/* ________________________________________ */
/* _____________ Parser tests ______________*/
/* ________________________________________ */

#define CLEANUP()                                                              \
    ll_free(tokens, (llfree_f)json_free_StringList);                               \
    json_entity_free(ent)

void parse_number(void) {
    char *_;
    StringList *tokens = json_tokenize_str("5", &_);
    StringList *rest;
    JSON_ENTITY *ent = json_parse_value(tokens, &rest);
    TEST_CHECK(json_to_double(ent) == (double) 5);
    CLEANUP();
}

void parse_bool(void) {
    char *_;
    StringList *tokens = json_tokenize_str("true", &_);
    StringList *rest;
    JSON_ENTITY *ent = json_parse_value(tokens, &rest);
    TEST_CHECK(json_to_bool(ent) == true);
    CLEANUP();
}

void parse_array(void) {
    char *_;
    StringList *tokens = json_tokenize_str("[true, 5, false, -2.27]", &_);
    json_type arr_types[] = {JSON_BOOL, JSON_NUM, JSON_BOOL, JSON_NUM};
    StringList *rest;
    JSON_ENTITY *ent = json_parse_value(tokens, &rest);
    TEST_CHECK(ent->type == JSON_ARRAY); /* type should be array */
    int len = json_get_arr_length(ent);
    TEST_CHECK(len == 4); /* length should be 4 */
    for (int i = 0; i < len; i++) {
        TEST_CHECK(json_get(ent, i)->type == arr_types[i]);
    }
    CLEANUP();
}

void parse_obj(void) {
    char *_;
    StringList *tokens = json_tokenize_str(
            "{\"foo\" : 5, \"bar\" : false, \"baz\" : [true]}", &_);
    char *expected_keys[] = {"\"foo\"", "\"bar\"", "\"baz\""};
    StringList *rest;
    JSON_ENTITY *ent = json_parse_value(tokens, &rest);
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

    char *_;
    StringList *tokens = json_tokenize_str(json, &_);
    char *expected_keys[] = {"\"foo\"", "\"bar\"", "\"baz\""};
    StringList *rest;
    JSON_ENTITY *ent = json_parse_value(tokens, &rest);
    TEST_CHECK(ent);
    CLEANUP();
}


#ifndef ACUTEST_H

struct test_ {
    const char *name;

    void (*func)(void);
};

#define TEST_LIST const struct test_ test_list_[]
#endif

TEST_LIST = {{"tokenize_true",       tokenize_true},
             {"tokenize_false",      tokenize_false},
             {"tokenize_null",       tokenize_null},
             {"tokenize_lbrace",     tokenize_lbrace},
             {"tokenize_rbrace",     tokenize_rbrace},
             {"tokenize_colon",      tokenize_colon},
             {"tokenize_lbracket",   tokenize_lbracket},
             {"tokenize_rbracket",   tokenize_rbracket},
             {"tokenize_comma",      tokenize_comma},
             {"tokenize_number",     tokenize_number},
             {"tokenize_big_string",     tokenize_big_string},
             {"tokenize_string",     tokenize_string},
             {"tokenize_whitespace", tokenize_whitespace},
             {"tokenize_multiword",  tokenize_multiword},
             {"tokenize_invalid",    tokenize_invalid},
        /* ________________________________________ */
             {"parse_number",        parse_number},
             {"parse_bool",          parse_bool},
             {"parse_array",         parse_array},
             {"parse_obj",           parse_obj},
             {"parse_obj_complex",   parse_obj_complex},
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
