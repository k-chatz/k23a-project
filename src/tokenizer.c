#include "../include/tokenizer.h"

static tokenizer_t *tokenizer_new(FILE *f, next_token_f next) {
    tokenizer_t *new = malloc(sizeof(*new));
    new->next = next;
    new->feof = false;
    new->f = f;
    new->buf_sz = 10;
    new->buf = malloc(10 * sizeof(char));
    return new;
}

tokenizer_t *tokenizer_new_from_filename(char *filename, next_token_f next) {
    FILE *f = fopen(filename, "r");
    tokenizer_t *new = tokenizer_new(f, next);
    return new;
}

tokenizer_t *tokenizer_new_from_string(char *string, next_token_f next) {
    FILE *f = fmemopen(string, strlen(string), "r");
    if (f == NULL)
        return NULL;

    tokenizer_t *new = tokenizer_new(f, next);

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

static inline bool is_stopword(tokenizer_t *tokenizer) {
    char *stopwords[148] = {
            "able", "about", "across", "after", "all", "almost", "also", "am", "among", "an", "and",
            "any", "are", "as", "at", "be", "because", "been", "but", "by", "can", "cannot", "could",
            "dear", "did", "do", "does", "either", "else", "ever", "every", "for", "from", "get", "got",
            "had", "has", "have", "he", "her", "hers", "him", "his", "how", "however", "i", "if", "in",
            "into", "is", "it", "its", "just", "least", "let", "like", "likely", "may", "me", "might",
            "most", "must", "my", "neither", "no", "nor", "not", "of", "off", "often", "on", "only",
            "or", "other", "our", "own", "rather", "said", "say", "says", "she", "should", "since",
            "so", "some", "than", "that", "the", "their", "them", "then", "there", "these", "they",
            "this", "tis", "to", "too", "twas", "us", "wants", "was", "we", "were", "what", "when",
            "where", "which", "while", "who", "whom", "why", "will", "with", "would", "yet", "you",
            "your", "mm", "f", "x", "a", "b", "c", "d", "e", "g", "h", "i", "j", "k", "l", "m", "n", "o",
            "p", "q", "r", "s", "t", "u", "v", "w", "x", "y", "z", "type", "mp"
    };

    bool flag = true;
    for (int j = 0; j < sizeof(stopwords); ++j) {
        flag = true;
        char *tok = stopwords[j];
        for (int i = 0; i < sizeof(tok) / sizeof(char) - 1; i++) {
            char ch = buf_get_char(tokenizer, i);
            if (ch != tok[i]) {
                flag = false;
                break;
            } else {
                if ((int) ch == 32) {
                    flag = true;
                    break;
                }
                printf("1");
            }
        }
        if (flag == true) {
            break;
        }
    }
    return flag;
}

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

#define RETURN_IF_TRUE(f)                                                      \
    if (f(tok))                                                                \
    return tok->buf

char *json_next_token(tokenizer_t *tok) {
    tok->buf[0] = '\0'; /* reset the token */

    /* eat whitespace first */
    while (isspace(buf_get_char(tok, 0))) {
        tok->buf[0] = '\0';
    }
    tok->buf[1] = '\0'; //    aaew//qwdq
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

char *str_next_token(tokenizer_t *tok) {
    tok->buf[0] = '\0'; /* reset the token */
    /* eat whitespace first */
    char ch;
    while (isspace((ch = buf_get_char(tok, 0))))
        tok->buf[0] = '\0';
    tok->buf[1] = '\0';

    if (is_stopword(tok)) {
        return tok->buf;

    }                                                             \
    //RETURN_IF_TRUE(is_number);
    return NULL; /* no valid token found */
}

tokenizer_t *json_tokenizer_from_filename(char *filename) {
    return tokenizer_new_from_filename(filename, json_next_token);
}

tokenizer_t *json_tokenizer_from_string(char *string) {
    return tokenizer_new_from_string(string, json_next_token);
}

tokenizer_t *tokenizer_from_string(char *string) {
    return tokenizer_new_from_string(string, str_next_token);
}

