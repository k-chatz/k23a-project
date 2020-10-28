#include "acutest.h"
#include "json_parser.h"
#include "stdbool.h"
#include <string.h>

#ifndef ACUTEST_H
#include <assert.h>
#define TEST_CHECK assert
#endif

#define ARR_LEN(ARR) (sizeof(ARR) / sizeof(ARR[0]))

bool tokenize_word(char *word) {
    StrList *tokens;
    tokens = json_tokenize_str(word);
    return (lllen(tokens) == 1) && (strcmp(tokens->data, word) == 0);
}

bool tokenize_sentence(char *str, char **expected_tokens) {
    StrList *tokens;
    tokens = json_tokenize_str(str);
    int i = 0;
    for (StrList *tok = tokens; tok; tok = llnth(tok, 1)) {
        if (expected_tokens[i] == NULL)
            return false;
        if (strcmp(tok->data, expected_tokens[i++]))
            return false;
    }
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

    str_valid digits[] = {{"0", true},
                          {"000", true},
                          {"123", true},
                          {"500", true},
                          {"1238904", true}};

    str_valid ints[] = {
        {"0", true},      {"1%1$s", true},  {"2%1$s", true},  {"3%1$s", true},
        {"4%1$s", true},  {"5%1$s", true},  {"6%1$s", true},  {"7%1$s", true},
        {"8%1$s", true},  {"9%1$s", true},  {"-0", true},     {"-1%1$s", true},
        {"-2%1$s", true}, {"-3%1$s", true}, {"-4%1$s", true}, {"-5%1$s", true},
        {"-6%1$s", true}, {"-7%1$s", true}, {"-8%1$s", true}, {"-9%1$s", true},
        {"01", false},    {"-01", false},   {"--5", false}};

    str_valid frac[] = {{"", true},
                        {".%2$s", true},
                        {".", false},
                        {".5.1", false},
                        {"..", false}};

    str_valid exp[] = {
        {"", true},      {"E%3$s", true},  {"E+%3$s", true}, {"E-%3$s", true},
        {"e%3$s", true}, {"e+%3$s", true}, {"e-%3$s", true}, {"e", false},
        {"e+", false},   {"e-", false},    {"E", false},     {"E+", false},
        {"E-", false},
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

void tokenize_whitespace(void) {
    /* tokenizing whitespace should yield no tokens */
    StrList *toks = json_tokenize_str("  \t \n ");
    TEST_CHECK(!toks);
}

void tokenize_multiword(void){
    char *in_toks[] = {"5e2", "true", "false"};
    char *in_str = "5e2true   \t\n\n false";
    char *out_toks[ARR_LEN(in_toks)];
    StrList *toks = json_tokenize_str(in_str);
    int i = 0;
    LLFOREACH(tok, toks){
	out_toks[i++] = tok->data;
    }
    for(i = 0; i < ARR_LEN(in_toks); i++){
	TEST_CHECK(strcmp(in_toks[i], out_toks[i]) == 0);
    }
}

#ifndef ACUTEST_H
struct test_ {
    const char *name;
    void (*func)(void);
};

#define TEST_LIST const struct test_ test_list_[]
#endif

TEST_LIST = {{"tokenize_true", tokenize_true},
             {"tokenize_false", tokenize_false},
             {"tokenize_null", tokenize_null},
             {"tokenize_lbrace", tokenize_lbrace},
             {"tokenize_rbrace", tokenize_rbrace},
             {"tokenize_colon", tokenize_colon},
             {"tokenize_lbracket", tokenize_lbracket},
             {"tokenize_rbracket", tokenize_rbracket},
             {"tokenize_comma", tokenize_comma},
             {"tokenize_number", tokenize_number},
             {"tokenize_string", tokenize_string},
             {"tokenize_whitespace", tokenize_whitespace},
	     {"tokenize_multiword", tokenize_multiword},
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
