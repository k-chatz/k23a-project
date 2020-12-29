#include "../include/tokenizer.h"

static tokenizer_t *tokenizer_new(FILE *f, setp stopwords, next_token_f next) {
    tokenizer_t *new = malloc(sizeof(*new));
    new->next = next;
    new->feof = false;
    new->f = f;
    new->stopwords = stopwords;
    new->buf_sz = 10;
    new->buf = malloc(10 * sizeof(char));
    return new;
}

tokenizer_t *tokenizer_new_from_filename(char *filename, setp stopwords, next_token_f next) {
    FILE *f = fopen(filename, "r");
    tokenizer_t *new = tokenizer_new(f, stopwords, next);
    return new;
}

tokenizer_t *tokenizer_new_from_string(char *string, setp stopwords, next_token_f next) {
    FILE *f = fmemopen(string, strlen(string), "r");
    if (f == NULL)
        return NULL;
    tokenizer_t *new = tokenizer_new(f, stopwords, next);
    return new;
}

void tokenizer_free(tokenizer_t *tok) {
    free(tok->buf);
    fclose(tok->f);
    free(tok);
}

char *tokenizer_next(tokenizer_t *tok) { return tok->next(tok); }

static inline void buf_set_char(tokenizer_t *tok, int i, char c) {
    if (i + 1 >= tok->buf_sz) {
        tok->buf_sz *= 2;
        tok->buf = realloc(tok->buf, tok->buf_sz * sizeof(char));
    }
    tok->buf[i] = c;
}


static char buf_get_char(tokenizer_t *tok, int i) __attribute__ ((hot));

static inline char buf_get_char(tokenizer_t *tok, int i) {
    if (tok->buf[i] == '\0' && !tok->feof) {
        int c = fgetc(tok->f);
        if (c == EOF) {
            tok->feof = true;
            return '\0';
        }
        buf_set_char(tok, i, (c > 0 ? c : 0));
        buf_set_char(tok, i + 1, '\0');
    }
    return tok->buf[i];
}

/* ________________________ */
/* Specific tokenizer rules */
/* ________________________ */

#define MATCH_TOKEN(FNAME, TOK)                                                \
    static inline bool FNAME(tokenizer_t *tokenizer) {                         \
        int i;                                                                 \
        for (i = 0; i < sizeof(TOK) / sizeof(char) - 1; i++) {                 \
            if (buf_get_char(tokenizer, i) != TOK[i])                          \
                return false;                                                  \
        }                                                                      \
        return true;                                                           \
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

static bool is_number(tokenizer_t *tokenizer) {
    size_t total_size = 0;
    /* INTEGER */
    if (buf_get_char(tokenizer, 0) == '-')
        total_size++;

    if (buf_get_char(tokenizer, total_size) == '0') {
        total_size++;
    } else if (isdigit(buf_get_char(tokenizer, total_size))) {
        do {
            total_size++;
        } while (isdigit(buf_get_char(tokenizer, total_size)));
    } else {
        /* this is not a number */
        return false;
    }
    /* FRACTION */
    if (buf_get_char(tokenizer, total_size) == '.') {
        size_t frac_size = 1;
        while (isdigit(buf_get_char(tokenizer, total_size + frac_size))) {
            frac_size++;
        }

        if (frac_size == 1) {
            return false;
        }

        total_size += frac_size;
    }
    /* EXPONENT */
    if (tolower(buf_get_char(tokenizer, total_size)) == 'e') {
        size_t exp_size = 1;
        char c = buf_get_char(tokenizer, total_size + exp_size);
        if (c == '+' || c == '-')
            exp_size++;

        while (isdigit(buf_get_char(tokenizer, total_size + exp_size))) {
            exp_size++;
        }

        if (!isdigit(buf_get_char(tokenizer, total_size + exp_size - 1)))
            return false;

        total_size += exp_size;
    }

    ungetc(buf_get_char(tokenizer, total_size), tokenizer->f);
    buf_set_char(tokenizer, total_size, '\0');
    return true;
}

static bool is_string(tokenizer_t *tokenizer) {
    int curr = 1;
    if (buf_get_char(tokenizer, 0) == '"') {
        while (buf_get_char(tokenizer, curr) != '"') {
            if (buf_get_char(tokenizer, curr) == '\\') {
                /* escape */
                switch (buf_get_char(tokenizer, curr + 1)) {
                    case 'u':
                        buf_get_char(tokenizer, curr + 2);
                        buf_get_char(tokenizer, curr + 3);
                        buf_get_char(tokenizer, curr + 4);
                        buf_get_char(tokenizer, curr + 5);
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
        if (buf_get_char(tokenizer, curr) == '"') {
            return true;
            buf_set_char(tokenizer, curr + 1, '\0');
        }
    }
    return false;
}

static inline bool is_accepted_nlp_word(tokenizer_t *tokenizer) {
    char ch;
    int i = 0;
    bool w = false;
    bool is_special;
    do {
        ch = buf_get_char(tokenizer, i);
        buf_set_char(tokenizer, i, (char) tolower(ch));
        is_special = isspace(ch) || ispunct(ch) || isdigit(ch);
        if (ch != 0 && is_special) {
            w = true;
        }
        i++;
    } while (ch != 0 && !is_special);
    buf_set_char(tokenizer, i - 1, 0);
    return w;
}

#define RETURN_IF_TRUE(f)                                                      \
    if (f(tok))                                                                \
    return tok->buf

char *json_next_token(tokenizer_t *tok) {
    tok->buf[0] = '\0'; /* reset the token */

    /* eat whitespace first */
    while (isspace(buf_get_char(tok, 0))) {
        tok->buf[0] = '\0';
    }
    tok->buf[1] = '\0';
    RETURN_IF_TRUE(is_true);
    RETURN_IF_TRUE(is_false);
    RETURN_IF_TRUE(is_null);
    RETURN_IF_TRUE(is_lbrace);
    RETURN_IF_TRUE(is_rbrace);
    RETURN_IF_TRUE(is_comma);
    RETURN_IF_TRUE(is_colon);
    RETURN_IF_TRUE(is_lbracket);
    RETURN_IF_TRUE(is_rbracket);
    RETURN_IF_TRUE(is_number);
    RETURN_IF_TRUE(is_string);
    return NULL; /* no valid token found */
}

char *str_next_nlp_token(tokenizer_t *tok) {
    tok->buf[0] = '\0'; /* reset the token */
    /* eat whitespace first */
    char ch;
    while (isspace((ch = buf_get_char(tok, 0))) || ispunct(ch) || isdigit(ch)) {
        tok->buf[0] = '\0';
    }
    tok->buf[1] = '\0';
    if (is_accepted_nlp_word(tok)) {
        //TODO: if (word <= 3 || word > 9) -> REJECT
        while (set_in(tok->stopwords, tok->buf)) {
            str_next_nlp_token(tok);
        }
        return tok->buf;
    }
    return NULL; /* no valid token found */
}

tokenizer_t *json_tokenizer_from_filename(char *filename) {
    return tokenizer_new_from_filename(filename, NULL, json_next_token);
}

tokenizer_t *json_tokenizer_from_string(char *string) {
    return tokenizer_new_from_string(string, NULL, json_next_token);
}

tokenizer_t *tokenizer_nlp(char *string, setp stopwords) {
    return tokenizer_new_from_string(string, stopwords, str_next_nlp_token);
}
