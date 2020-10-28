#include "acutest.h"
#include "json_parser.h"
#include "stdbool.h"

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
                            sprintf(buf2, buf1, digits[d1], digits[d2],
                                    digits[d3]);
                            bool success = tokenize_word(buf2);
                            TEST_CHECK(success == expected_success);
                        }
                    }
                }
            }
        }
    }
}


