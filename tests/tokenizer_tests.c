#include <stdbool.h>

#include "../include/acutest.h"
#include "../include/tokenizer.h"

#define ARR_LEN(ARR) (sizeof(ARR) / sizeof(ARR[0]))

bool tokenize_word(char *word) {
    tokenizer_t *tokenizer = json_tokenizer_from_string(word);

    char *token = tokenizer_next(tokenizer);
    bool success = token != NULL && (strcmp(token, word) == 0);
    success = success && (tokenizer_next(tokenizer) == NULL);

    tokenizer_free(tokenizer);
    return success;
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
                            sprintf(buf2, buf1, digits[d1].str, digits[d2].str,
                                    digits[d3].str);
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
    // char *str_contents[] = {"fhasdj", "foo bar", "foo 123bar _@@!@#$%^&*()_+/", "\\\"", "\\uAbFf"};
    char *str_contents[] = {"foo 123bar _@@!@#$%^&*()_+/"};
    char buf[100];
    for (int i = 0; i < ARR_LEN(str_contents); i++) {
        sprintf(buf, "\"%s\"", str_contents[i]);
                TEST_CHECK(tokenize_word(buf));
    }
}

void tokenize_big_string(void) {
    char *bigstr =
            "\"Used: An item that has been used previously. The item may have some "
            "signs of cosmetic wear, but is fully\noperational and functions as "
            "intended. This item may be a floor model or store return that has "
            "been used. See the seller\\u00e2\\u0080\\u0099s listing for full "
            "details and description of any imperfections.\nSee all condition "
            "definitions- opens in a new window or tab\n... Read moreabout the "
            "condition\"";

            TEST_CHECK(tokenize_word(bigstr));
}

void tokenize_whitespace(void) {
    /* tokenizing whitespace should yield no tokens and consume it */
    tokenizer_t *tok = json_tokenizer_from_string("  \t\n \n  ");
            TEST_CHECK(tokenizer_next(tok) == NULL && tok->feof);
    tokenizer_free(tok);
}

void tokenize_sentence(void) {
    /* tokenizing whitespace should yield no tokens and consume it */
    tokenizer_t *tok = tokenizer_from_string(
            "    this is a string, is    true is a   string is   able to trueueueue "); // [a-zA-z]+
    char *token = NULL;

   while ((token = tokenizer_next(tok)) != NULL) {
       printf("[%s]", token);
   }
    TEST_CHECK(1);
    tokenizer_free(tok);
}

void tokenize_multiword(void) {
    char *in_toks[] = {"5e2", "true", "false"};
    char *in_str = "5e2true   \t\n\n false";
    tokenizer_t *tokenizer = json_tokenizer_from_string(in_str);
    for (int i = 0; i < ARR_LEN(in_toks); i++) {
        char *out_tok = tokenizer_next(tokenizer);
                TEST_CHECK(strcmp(in_toks[i], out_tok) == 0);
    }

    /* check if we consumed all the input */
            TEST_CHECK(tokenizer_next(tokenizer) == NULL);
            TEST_CHECK(tokenizer->feof);
    tokenizer_free(tokenizer);
}

TEST_LIST = {
        {"tokenize_true",       tokenize_true},
        {"tokenize_false",      tokenize_false},
        {"tokenize_null",       tokenize_null},
        {"tokenize_lbrace",     tokenize_lbrace},
        {"tokenize_rbrace",     tokenize_rbrace},
        {"tokenize_colon",      tokenize_colon},
        {"tokenize_lbracket",   tokenize_lbracket},
        {"tokenize_rbracket",   tokenize_rbracket},
        {"tokenize_comma",      tokenize_comma},
        {"tokenize_number",     tokenize_number},
        {"tokenize_string",     tokenize_string},
        {"tokenize_big_string", tokenize_big_string},
        {"tokenize_whitespace", tokenize_whitespace},
        // {"tokenize_sentence",   tokenize_sentence},
        {"tokenize_multiword",  tokenize_multiword},
        {NULL, NULL}
};

