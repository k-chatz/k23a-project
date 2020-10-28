#include "json_parser.h"
#include <string.h>
#include <ctype.h>

#define ARR_LEN(ARR) (sizeof(ARR) / sizeof(ARR[0]))
#define DIGITS "0123456789"

typedef char *(*matcher)(char *input);

#define MATCH_TOKEN(FNAME, TOK)                                                \
    static char *FNAME(char *input) {                                          \
        int cmp = strncmp(input, TOK, strlen(TOK));                            \
        if (!cmp)                                                              \
            return TOK;                                                        \
        return "";                                                             \
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
	if(input[curr] == '"'){
	    char *out = strndup(input, curr+1);
	    return out;
	}
    }
    return strdup("");
}

static char *json_next_token(char *input) {
    matcher tokenizers[] = {is_true,     is_false,  is_null,  is_lbrace,
                            is_rbrace,   is_comma,  is_colon, is_lbracket,
                            is_rbracket, is_number, is_string, NULL};

    char *match;
    for (int i = 0; tokenizers[i] != NULL; i++) {
        match = tokenizers[i](input);
        if (strlen(match) > 0)
            return match;
    }

    return "";
}

static char *eat_ws(char *str){
    char *out = str;
    while(isspace(*out))
	out++;
    return out;
}

StrList *json_tokenize_str(char *str) {
    StrList *out = NULL;
    char *current;
    do {
	str = eat_ws(str);
        current = json_next_token(str);
        str += strlen(current);
	if(current[0] != '\0')
	    llpush(&out, create_node(current));
    } while (str[0] != '\0' && current[0] != '\0');

    llreverse(&out);
    return out;
}

JSON_ENTITY json_parse_from_tokens(StrList *tokens) { return NULL; }
