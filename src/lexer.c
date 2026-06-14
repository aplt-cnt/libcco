#include <cnt/cco_p.h>
#include <stdio.h>

/* ---------------------------------------------------------------------------
 * Keyword matching
 * -------------------------------------------------------------------------*/
typedef struct { const char *word; cco_token_kind_t kind; } kw_entry;

#define KW(w,k) { w, k }
static const kw_entry keyword_map[] = {
    KW("true",     CCO_TK_TRUE),
    KW("false",    CCO_TK_FALSE),
    KW("None",     CCO_TK_NONE),
    KW("String",   CCO_TK_STRING_TYPE),
    KW("Integer",  CCO_TK_INTEGER_TYPE),
    KW("Float",    CCO_TK_FLOAT_TYPE),
    KW("Boolean",  CCO_TK_BOOLEAN_TYPE),
    KW("Array",    CCO_TK_ARRAY),
    KW("dyn",      CCO_TK_DYN),
    KW("this",     CCO_TK_THIS),
};
#undef KW

static cco_token_kind_t lookup_keyword(const char *s, size_t len) {
    for (size_t i = 0; i < sizeof(keyword_map)/sizeof(keyword_map[0]); i++) {
        const char *kw = keyword_map[i].word;
        if (strlen(kw) == len && memcmp(s, kw, len) == 0)
            return keyword_map[i].kind;
    }
    return CCO_TK_IDENT;
}

static cco_token_kind_t lookup_dollar_keyword(const char *s, size_t len) {
    if (len == 7 && memcmp(s, "typedef", 7) == 0)  return CCO_TK_TYPEDEF;
    if (len == 4 && memcmp(s, "enum", 4) == 0)      return CCO_TK_ENUM;
    if (len == 4 && memcmp(s, "temp", 4) == 0)      return CCO_TK_TEMP;
    if (len == 8 && memcmp(s, "function", 8) == 0)  return CCO_TK_FUNCTION;
    if (len == 7 && memcmp(s, "default", 7) == 0)   return CCO_TK_DEFAULT;
    if (len == 4 && memcmp(s, "this", 4) == 0)      return CCO_TK_THIS_REF;
    if (len == 6 && memcmp(s, "return", 6) == 0)    return CCO_TK_RETURN;
    if (len == 6 && memcmp(s, "format", 6) == 0)    return CCO_TK_FORMAT;
    return CCO_TK_IDENT;
}

/* ---------------------------------------------------------------------------
 * Lexer helpers
 * -------------------------------------------------------------------------*/
static inline char peek_char(const cco_lexer_t *lx) {
    return lx->pos < lx->source.len ? lx->source.data[lx->pos] : '\0';
}

static inline char advance(cco_lexer_t *lx) {
    char c = lx->source.data[lx->pos++];
    if (c == '\n') { lx->line++; lx->col = 1; }
    else { lx->col++; }
    return c;
}

static void skip_whitespace_and_comments(cco_lexer_t *lx) {
    for (;;) {
        char c = peek_char(lx);
        if (c == ' ' || c == '\t' || c == '\n' || c == '\r') {
            advance(lx);
            continue;
        }
        if (c == '/') {
            if (lx->pos + 1 < lx->source.len && lx->source.data[lx->pos+1] == '*') {
                advance(lx); advance(lx); /* skip /* */
                while (lx->pos < lx->source.len) {
                    if (peek_char(lx) == '*' &&
                        lx->pos + 1 < lx->source.len &&
                        lx->source.data[lx->pos+1] == '/') {
                        advance(lx); advance(lx); /* skip */ break;
                    }
                    advance(lx);
                }
                continue;
            }
        }
        break;
    }
}

static cco_token_t make_token(cco_lexer_t *lx, cco_token_kind_t kind,
                              size_t start_pos, size_t len) {
    cco_token_t t;
    t.kind   = kind;
    t.start  = lx->source.data + start_pos;
    t.length = len;
    t.line   = lx->line;
    t.col    = lx->col - len;
    return t;
}

/* ---------------------------------------------------------------------------
 * Lexer API
 * -------------------------------------------------------------------------*/
void cco_lexer_init(cco_lexer_t *lx, const char *text,
                    const char *filename, cco_arena_t *arena) {
    lx->pos       = 0;
    lx->line      = 1;
    lx->col       = 1;
    lx->has_peek  = 0;
    lx->arena     = arena;
    lx->error_flag = 0;
    cco_source_init(&lx->source, text, filename);
    memset(&lx->current, 0, sizeof(lx->current));
    memset(&lx->peek, 0, sizeof(lx->peek));
}

int cco_lexer_ok(const cco_lexer_t *lx) {
    return !lx->error_flag;
}

cco_token_t cco_lexer_next(cco_lexer_t *lx) {
    if (lx->has_peek) {
        lx->has_peek = 0;
        lx->current  = lx->peek;
        return lx->current;
    }

    skip_whitespace_and_comments(lx);
    size_t start_pos = lx->pos;
    size_t start_col = lx->col;
    size_t start_line = lx->line;
    char c = peek_char(lx);

    if (c == '\0') {
        lx->current = make_token(lx, CCO_TK_EOF, start_pos, 0);
        return lx->current;
    }

    /* String literal */
    if (c == '"') {
        advance(lx);
        size_t str_start = lx->pos;
        while (lx->pos < lx->source.len) {
            if (peek_char(lx) == '"') {
                size_t len = lx->pos - str_start;
                advance(lx);
                lx->current = make_token(lx, CCO_TK_STRING_LIT, str_start, len);
                return lx->current;
            }
            if (peek_char(lx) == '\\') {
                advance(lx); /* skip escape char */
                if (lx->pos < lx->source.len) advance(lx);
            } else {
                advance(lx);
            }
        }
        lx->error_flag = 1;
        lx->current = make_token(lx, CCO_TK_ERROR, str_start, 0);
        return lx->current;
    }

    /* Raw string literal (backtick) */
    if (c == '`') {
        advance(lx);
        size_t str_start = lx->pos;
        while (lx->pos < lx->source.len && peek_char(lx) != '`')
            advance(lx);
        if (lx->pos >= lx->source.len) {
            lx->error_flag = 1;
            lx->current = make_token(lx, CCO_TK_ERROR, str_start, 0);
            return lx->current;
        }
        size_t len = lx->pos - str_start;
        advance(lx); /* skip closing backtick */
        lx->current = make_token(lx, CCO_TK_RAW_STRING_LIT, str_start, len);
        return lx->current;
    }

    /* Identifier or keyword */
    if (isalpha((unsigned char)c) || c == '_' || c == '$') {
        /* $-keyword: $identifier */
        if (c == '$') {
            advance(lx);
            size_t kw_start = lx->pos;
            while (isalnum((unsigned char)peek_char(lx)) || peek_char(lx) == '_')
                advance(lx);
            size_t kw_len = lx->pos - kw_start;
            cco_token_kind_t k = lookup_dollar_keyword(lx->source.data + kw_start, kw_len);
            lx->current = make_token(lx, k, kw_start - 1, kw_len + 1);
            return lx->current;
        }
        /* Regular identifier */
        while (isalnum((unsigned char)peek_char(lx)) || peek_char(lx) == '_')
            advance(lx);
        size_t id_len = lx->pos - start_pos;

        /* Check for anonymous param: _$ followed by digits */
        if (c == '_' && id_len == 1 && peek_char(lx) == '$') {
            advance(lx); /* consume $ */
            size_t digit_start = lx->pos;
            while (isdigit((unsigned char)peek_char(lx)))
                advance(lx);
            if (lx->pos > digit_start) {
                /* It's an anon param, e.g., _$0, _$123 */
                lx->current = make_token(lx, CCO_TK_ANON_PARAM, start_pos, lx->pos - start_pos);
                return lx->current;
            }
            /* Not an anon param: just '_$' with no digits is not valid.
               Revert? Actually we already consumed. Let's treat as error. */
            lx->error_flag = 1;
            lx->current = make_token(lx, CCO_TK_ERROR, start_pos, lx->pos - start_pos);
            return lx->current;
        }

        cco_token_kind_t k = lookup_keyword(lx->source.data + start_pos, id_len);
        lx->current = make_token(lx, k, start_pos, id_len);
        return lx->current;
    }

    /* Number */
    if (isdigit((unsigned char)c) || (c == '.' && lx->pos + 1 < lx->source.len &&
                                       isdigit((unsigned char)lx->source.data[lx->pos+1]))) {
        /* Check for hex/octal/binary prefix */
        int is_float = 0;
        if (c == '0' && lx->pos + 1 < lx->source.len) {
            char nxt = lx->source.data[lx->pos+1];
            if (nxt == 'x' || nxt == 'X' || nxt == 'o' || nxt == 'O' ||
                nxt == 'b' || nxt == 'B') {
                advance(lx); advance(lx); /* 0x */
                while (isxdigit((unsigned char)peek_char(lx)) || peek_char(lx) == '_')
                    advance(lx);
                goto number_done;
            }
        }
        /* Decimal/float */
        while (isdigit((unsigned char)peek_char(lx)) || peek_char(lx) == '_')
            advance(lx);
        if (peek_char(lx) == '.') {
            is_float = 1;
            advance(lx);
            while (isdigit((unsigned char)peek_char(lx)) || peek_char(lx) == '_')
                advance(lx);
        }
        if (peek_char(lx) == 'e' || peek_char(lx) == 'E') {
            is_float = 1;
            advance(lx);
            if (peek_char(lx) == '+' || peek_char(lx) == '-') advance(lx);
            while (isdigit((unsigned char)peek_char(lx)) || peek_char(lx) == '_')
                advance(lx);
        }
    number_done:
        lx->current = make_token(lx, is_float ? CCO_TK_FLOAT_LIT : CCO_TK_INT_LIT,
                                 start_pos, lx->pos - start_pos);
        return lx->current;
    }

    /* Anonymous parameter ($_ + digits) */
    /* Note: This starts with '_' then '$' then digits, which conflicts with
       identifier rule. We handle it in the identifier branch above since
       '_' starts an identifier. Actually the spec says ANON_PARAM is '_$' + digits.
       Since '_' goes through identifier parsing, after seeing '_$' we need to
       check if remaining chars are digits. Let's handle this after an identifier
       is parsed. */
    /* Check if we just parsed an identifier starting with '_' followed by '$digit': */
    /* We already handled this above in the identifier branch. Let's add a check there.
       Actually, let's handle it after the identifier token is created. */

    /* Operators and delimiters */
    advance(lx);
    switch (c) {
        case '(': lx->current = make_token(lx, CCO_TK_LPAREN, start_pos, 1); return lx->current;
        case ')': lx->current = make_token(lx, CCO_TK_RPAREN, start_pos, 1); return lx->current;
        case ':': lx->current = make_token(lx, CCO_TK_COLON,  start_pos, 1); return lx->current;
        case ',': lx->current = make_token(lx, CCO_TK_COMMA,  start_pos, 1); return lx->current;
        case '+': lx->current = make_token(lx, CCO_TK_PLUS,   start_pos, 1); return lx->current;
        case '-':
            if (peek_char(lx) == '>') { advance(lx);
                lx->current = make_token(lx, CCO_TK_ARROW, start_pos, 2); return lx->current; }
            lx->current = make_token(lx, CCO_TK_MINUS, start_pos, 1); return lx->current;
        case '*': lx->current = make_token(lx, CCO_TK_STAR,  start_pos, 1); return lx->current;
        case '/': lx->current = make_token(lx, CCO_TK_SLASH, start_pos, 1); return lx->current;
        case '=':
            if (peek_char(lx) == '=') { advance(lx);
                lx->current = make_token(lx, CCO_TK_EQEQ, start_pos, 2); return lx->current; }
            lx->current = make_token(lx, CCO_TK_EQUAL, start_pos, 1); return lx->current;
        case '!':
            if (peek_char(lx) == '=') { advance(lx);
                lx->current = make_token(lx, CCO_TK_NEQ, start_pos, 2); return lx->current; }
            lx->current = make_token(lx, CCO_TK_NOT, start_pos, 1); return lx->current;
        case '<':
            if (peek_char(lx) == '=') { advance(lx);
                lx->current = make_token(lx, CCO_TK_LTEQ, start_pos, 2); return lx->current; }
            lx->current = make_token(lx, CCO_TK_LT, start_pos, 1); return lx->current;
        case '>':
            if (peek_char(lx) == '=') { advance(lx);
                lx->current = make_token(lx, CCO_TK_GTEQ, start_pos, 2); return lx->current; }
            lx->current = make_token(lx, CCO_TK_GT, start_pos, 1); return lx->current;
        case '&':
            if (peek_char(lx) == '&') { advance(lx);
                lx->current = make_token(lx, CCO_TK_AND, start_pos, 2); return lx->current; }
            lx->error_flag = 1;
            lx->current = make_token(lx, CCO_TK_ERROR, start_pos, 1); return lx->current;
        case '|':
            if (peek_char(lx) == '|') { advance(lx);
                lx->current = make_token(lx, CCO_TK_OR, start_pos, 2); return lx->current; }
            lx->current = make_token(lx, CCO_TK_PIPE, start_pos, 1); return lx->current;
        case '.': lx->current = make_token(lx, CCO_TK_DOT,    start_pos, 1); return lx->current;
        case '#': lx->current = make_token(lx, CCO_TK_HASH,   start_pos, 1); return lx->current;
        case '@': lx->current = make_token(lx, CCO_TK_AT,     start_pos, 1); return lx->current;
        case '`': lx->current = make_token(lx, CCO_TK_BACKTICK, start_pos, 1); return lx->current;
        default:
            lx->error_flag = 1;
            lx->current = make_token(lx, CCO_TK_ERROR, start_pos, 1); return lx->current;
    }
}

cco_token_t cco_lexer_peek(cco_lexer_t *lx) {
    if (!lx->has_peek) {
        lx->peek = cco_lexer_next(lx);
        lx->has_peek = 1;
    }
    return lx->peek;
}
