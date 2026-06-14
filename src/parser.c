#include <cnt/cco_p.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>

/*============================================================================
 * Simple recursive-descent parser (value-layer, Phase 0)
 *
 * Grammar:
 *   File       ::= Value
 *   Value      ::= Object | Array | Literal
 *   Object     ::= '(' Field (',' Field)* ','? ')'
 *                | IDENTIFIER ':' Value          (top-level shorthand)
 *   Field      ::= IDENTIFIER ':' Value
 *   Array      ::= '(' Value (',' Value)* ','? ')'
 *   Literal    ::= STRING | RAW_STRING | INT | FLOAT
 *                | 'true' | 'false' | 'None'
 *============================================================================*/

typedef struct cco_parser {
    cco_lexer_t        lexer;
    cco_arena_t       *arena;
    int                error;
    cco_symbol_table_t *symtab;  /* NULL for value-only parsing */
} cco_parser_t;

static void parser_init(cco_parser_t *p, const char *text,
                        const char *filename, cco_arena_t *arena) {
    cco_lexer_init(&p->lexer, text, filename, arena);
    p->arena  = arena;
    p->error  = 0;
    p->symtab = NULL;
}

static cco_token_t next(cco_parser_t *p) {
    return cco_lexer_next(&p->lexer);
}

static cco_token_t peek(cco_parser_t *p) {
    return cco_lexer_peek(&p->lexer);
}

static int expect(cco_parser_t *p, cco_token_kind_t kind) {
    cco_token_t t = next(p);
    if (t.kind != kind) {
        p->error = 1;
        CCO_ERR_AT(p->lexer.source.filename, t.line, t.col,
                       "expected token kind %d, got %d", (int)kind, (int)t.kind);
        return 0;
    }
    return 1;
}

static int match(cco_parser_t *p, cco_token_kind_t kind) {
    if (peek(p).kind == kind) {
        next(p);
        return 1;
    }
    return 0;
}

/* Forward declaration */
static cco_object_t* parse_value(cco_parser_t *p);

/* Parse escape sequences in a string literal token */
static char* parse_string_literal(const cco_token_t *t, cco_arena_t *arena) {
    /* First pass: calculate result length */
    size_t dst_len = 0;
    for (size_t i = 0; i < t->length; i++) {
        if (t->start[i] == '\\' && i + 1 < t->length) {
            i++; /* skip escape char */
            dst_len++;
        } else {
            dst_len++;
        }
    }

    char *result = (char*)cco_arena_alloc(arena, dst_len + 1);
    if (!result) {
        result = (char*)malloc(dst_len + 1); /* fallback to heap */
        if (!result) return NULL;
        /* We'll leak this if arena alloc succeeds, but it's ok for now */
    }

    size_t j = 0;
    for (size_t i = 0; i < t->length; i++) {
        if (t->start[i] == '\\' && i + 1 < t->length) {
            i++;
            switch (t->start[i]) {
                case 'n':  result[j++] = '\n'; break;
                case 'r':  result[j++] = '\r'; break;
                case 't':  result[j++] = '\t'; break;
                case '\\': result[j++] = '\\'; break;
                case '"':  result[j++] = '"';  break;
                case 'x': {
                    if (i + 2 < t->length) {
                        char hex[3] = { t->start[i+1], t->start[i+2], '\0' };
                        result[j++] = (char)strtol(hex, NULL, 16);
                        i += 2;
                    }
                    break;
                }
                case 'u': {
                    if (i + 4 < t->length) {
                        char hex[5] = { t->start[i+1], t->start[i+2],
                                        t->start[i+3], t->start[i+4], '\0' };
                        unsigned long cp = strtoul(hex, NULL, 16);
                        if (cp < 0x80) {
                            result[j++] = (char)cp;
                        } else if (cp < 0x800) {
                            result[j++] = (char)(0xC0 | (cp >> 6));
                            result[j++] = (char)(0x80 | (cp & 0x3F));
                        } else {
                            result[j++] = (char)(0xE0 | (cp >> 12));
                            result[j++] = (char)(0x80 | ((cp >> 6) & 0x3F));
                            result[j++] = (char)(0x80 | (cp & 0x3F));
                        }
                        i += 4;
                    }
                    break;
                }
                default: result[j++] = t->start[i]; break;
            }
        } else {
            result[j++] = t->start[i];
        }
    }
    result[j] = '\0';
    return result;
}

static char* extract_token_text(const cco_token_t *t, cco_arena_t *arena) {
    char *s = (char*)cco_arena_alloc(arena, t->length + 1);
    if (!s) s = (char*)malloc(t->length + 1);
    if (!s) return NULL;
    memcpy(s, t->start, t->length);
    s[t->length] = '\0';
    return s;
}

static cco_object_t* parse_literal(cco_parser_t *p, cco_token_t t) {
    switch (t.kind) {
    case CCO_TK_STRING_LIT: {
        char *s = parse_string_literal(&t, p->arena);
        if (!s) { p->error = 1; return NULL; }
        return cco_string_new(s);
    }
    case CCO_TK_RAW_STRING_LIT: {
        char *s = extract_token_text(&t, p->arena);
        if (!s) { p->error = 1; return NULL; }
        return cco_string_new(s);
    }
    case CCO_TK_INT_LIT: {
        /* Remove underscores */
        char buf[64];
        size_t j = 0;
        for (size_t i = 0; i < t.length && j < sizeof(buf) - 1; i++) {
            if (t.start[i] != '_') buf[j++] = t.start[i];
        }
        buf[j] = '\0';

        long long val;
        /* Handle 0x, 0o, 0b prefixes that strtoll might not support */
        if (j >= 2 && buf[0] == '0') {
            if (buf[1] == 'x' || buf[1] == 'X')
                val = strtoll(buf + 2, NULL, 16);
            else if (buf[1] == 'o' || buf[1] == 'O')
                val = strtoll(buf + 2, NULL, 8);
            else if (buf[1] == 'b' || buf[1] == 'B')
                val = strtoll(buf + 2, NULL, 2);
            else
                val = strtoll(buf, NULL, 0);
        } else {
            val = strtoll(buf, NULL, 10);
        }
        return cco_int_new(val);
    }
    case CCO_TK_FLOAT_LIT: {
        char buf[64];
        size_t j = 0;
        for (size_t i = 0; i < t.length && j < sizeof(buf) - 1; i++) {
            if (t.start[i] != '_') buf[j++] = t.start[i];
        }
        buf[j] = '\0';
        double val = strtod(buf, NULL);
        return cco_float_new(val);
    }
    case CCO_TK_TRUE:  return cco_bool_new(1);
    case CCO_TK_FALSE: return cco_bool_new(0);
    case CCO_TK_NONE:  return cco_none_new();
    default:
        p->error = 1;
        LEX_ERR(&p->lexer, "unexpected token at line %zu col %zu", t.line, t.col);
        return NULL;
    }
}

static cco_object_t* parse_object(cco_parser_t *p) {
    /* Already consumed '(' */
    cco_object_t *obj = cco_object_new();
    if (!obj) { p->error = 1; return NULL; }

    /* Check for empty object */
    if (match(p, CCO_TK_RPAREN))
        return obj;

    for (;;) {
        cco_token_t key_tok = next(p);
        if (key_tok.kind != CCO_TK_IDENT) {
            p->error = 1;
            LEX_ERR(&p->lexer, "expected field name at line %zu col %zu",
                           key_tok.line, key_tok.col);
            cco_object_free(obj);
            return NULL;
        }
        char *key = extract_token_text(&key_tok, p->arena);
        if (!key) { p->error = 1; cco_object_free(obj); return NULL; }

        if (!expect(p, CCO_TK_COLON)) { cco_object_free(obj); return NULL; }

        cco_object_t *val = parse_value(p);
        if (!val || p->error) { cco_object_free(obj); cco_object_free(val); return NULL; }

        /* key is arena-allocated, but cco_object_set expects malloc'd key.
           Actually value.c's safe_strdup does strdup. Let's just use set. */
        cco_object_set(obj, key, val);

        if (match(p, CCO_TK_RPAREN)) break;
        if (!match(p, CCO_TK_COMMA)) {
            /* Try to recover - if next is RPAREN, accept */
            if (peek(p).kind == CCO_TK_RPAREN) { next(p); break; }
            p->error = 1;
            LEX_ERR(&p->lexer, "expected ',' or ')' at line %zu col %zu",
                           peek(p).line, peek(p).col);
            cco_object_free(obj);
            return NULL;
        }
        if (peek(p).kind == CCO_TK_RPAREN) { next(p); break; }
    }
    return obj;
}

static cco_object_t* parse_array(cco_parser_t *p) {
    /* Already consumed '(' */
    cco_array_t *arr = cco_array_new();
    if (!arr) { p->error = 1; return NULL; }

    if (match(p, CCO_TK_RPAREN))
        return cco_array_wrap(arr);

    for (;;) {
        cco_object_t *val = parse_value(p);
        if (!val || p->error) { cco_array_free(arr); cco_object_free(val); return NULL; }
        cco_array_add(arr, val);

        if (match(p, CCO_TK_RPAREN)) break;
        if (!match(p, CCO_TK_COMMA)) {
            if (peek(p).kind == CCO_TK_RPAREN) { next(p); break; }
            p->error = 1;
            LEX_ERR(&p->lexer, "expected ',' or ')' in array at line %zu col %zu",
                           peek(p).line, peek(p).col);
            cco_array_free(arr);
            return NULL;
        }
        if (peek(p).kind == CCO_TK_RPAREN) { next(p); break; }
    }
    return cco_array_wrap(arr);
}

static cco_object_t* parse_value(cco_parser_t *p) {
    cco_token_t t = peek(p);

    /* Object or Array: '(' ... ')' */
    if (t.kind == CCO_TK_LPAREN) {
        next(p); /* consume '(' */

        /* Disambiguate: if next token is IDENTIFIER followed by ':', it's an object */
        cco_token_t after_paren = peek(p);
        if (after_paren.kind == CCO_TK_IDENT) {
            /* Peek one more: if followed by ':', it's an object */
            /* We need a 2-token lookahead. Since our lexer only supports 1,
               we'll use a simple trick: parse as object, and if it fails,
               backtrack. But that's complex. Instead, let's just try object
               format: if peek after IDENT is COLON, it's object. */
            /* Actually peek only gives 1 token. Let me use the lexer's
               internal peek mechanism. We can temporarily advance. */
            /* Simple approach: consume the IDENT, check if next is ':' */
            cco_token_t id_tok = next(p);
            /* Check for optional <annotation> after field name */
            if (peek(p).kind == CCO_TK_LT) {
                int depth = 1; next(p);
                while (depth > 0 && peek(p).kind != CCO_TK_EOF) {
                    cco_token_t at2 = next(p);
                    if (at2.kind == CCO_TK_LT) depth++;
                    else if (at2.kind == CCO_TK_GT) depth--;
                }
            }
            if (peek(p).kind == CCO_TK_COLON) {
                /* It's an object field: key: value */
                next(p);
                cco_object_t *obj = cco_object_new();
                if (!obj) { p->error = 1; return NULL; }
                char *key = extract_token_text(&id_tok, p->arena);
                if (!key) { p->error = 1; cco_object_free(obj); return NULL; }
                cco_object_t *val = parse_value(p);
                if (!val || p->error) { cco_object_free(obj); cco_object_free(val); return NULL; }
                cco_object_set(obj, key, val);
                /* Continue parsing remaining fields */
                for (;;) {
                    if (match(p, CCO_TK_RPAREN)) break;
                    if (!match(p, CCO_TK_COMMA)) {
                        if (peek(p).kind == CCO_TK_RPAREN) { next(p); break; }
                        p->error = 1;
                        LEX_ERR(&p->lexer, "expected ',' or ')' at line %zu col %zu",
                                       peek(p).line, peek(p).col);
                        cco_object_free(obj);
                        return NULL;
                    }
                    if (peek(p).kind == CCO_TK_RPAREN) { next(p); break; }
                    cco_token_t fk = next(p);
                    if (fk.kind != CCO_TK_IDENT) {
                        p->error = 1;
                        cco_object_free(obj);
                        return NULL;
                    }
                    char *fn = extract_token_text(&fk, p->arena);
                    if (!expect(p, CCO_TK_COLON)) { cco_object_free(obj); return NULL; }
                    cco_object_t *fv = parse_value(p);
                    if (!fv || p->error) { cco_object_free(obj); cco_object_free(fv); return NULL; }
                    cco_object_set(obj, fn, fv);
                }
                return obj;
            } else {
                /* It's an array - but we already consumed the first element as id_tok */
                cco_array_t *arr = cco_array_new();
                if (!arr) { p->error = 1; return NULL; }
                /* parse id_tok as literal identifier */
                /* Actually in the value layer, an identifier in an array is an error
                   unless it's a keyword (true/false/None), which id_tok isn't since
                   those would return CCO_TK_TRUE etc. Let's treat as string literal */
                char *s = extract_token_text(&id_tok, p->arena);
                cco_array_add(arr, cco_string_new(s));
                /* Continue parsing array */
                for (;;) {
                    if (match(p, CCO_TK_RPAREN)) break;
                    if (!match(p, CCO_TK_COMMA)) {
                        if (peek(p).kind == CCO_TK_RPAREN) { next(p); break; }
                        p->error = 1;
                        cco_array_free(arr);
                        return NULL;
                    }
                    if (peek(p).kind == CCO_TK_RPAREN) { next(p); break; }
                    cco_object_t *av = parse_value(p);
                    if (!av || p->error) { cco_array_free(arr); cco_object_free(av); return NULL; }
                    cco_array_add(arr, av);
                }
                return cco_array_wrap(arr);
            }
        } else if (after_paren.kind == CCO_TK_RPAREN) {
            /* Empty parens - ambiguous. Default to empty object. */
            next(p);
            return cco_object_new();
        } else {
            /* It's an array */
            return parse_array(p);
        }
    }

    /* Top-level shorthand: IDENTIFIER ':' Value (only at file root) */
    /* Handled by caller */

    /* Dot-notation static call: TypeName.method(args).
       For now, only IDENT when followed by LPAREN or COLON enters the
       value/shorthand path. IDENT . IDENT ( should be handled as a
       static call. This requires 3-token lookahead which the current
       lexer doesn't support. For now, use #Type:method() instead. */

    /* Template instantiation: #TemplateName(args) or #TemplateName:method(args) */
    if (t.kind == CCO_TK_HASH) {
        next(p); /* consume # */
        cco_token_t name_tok = next(p);
        if (name_tok.kind != CCO_TK_IDENT) {
            p->error = 1;
            LEX_ERR(&p->lexer, "expected template name after '#' at line %zu col %zu",
                           name_tok.line, name_tok.col);
            return NULL;
        }
        char *tname = extract_token_text(&name_tok, p->arena);

        /* Optional generic type args: #Name<Type>(args) — skip < > */
        if (peek(p).kind == CCO_TK_LT) {
            int depth = 1; next(p);
            while (depth > 0 && peek(p).kind != CCO_TK_EOF) {
                cco_token_t gt = next(p);
                if (gt.kind == CCO_TK_LT) depth++;
                else if (gt.kind == CCO_TK_GT) depth--;
            }
        }

        /* Static method call: #Name:method(args) */
        if (peek(p).kind == CCO_TK_COLON) {
            next(p); /* consume : */
            cco_token_t method_tok = next(p);
            if (method_tok.kind != CCO_TK_IDENT) {
                p->error = 1;
                LEX_ERR(&p->lexer, "expected method name after ':' at line %zu col %zu",
                               method_tok.line, method_tok.col);
                return NULL;
            }
            char *mname = extract_token_text(&method_tok, p->arena);
            if (!expect(p, CCO_TK_LPAREN)) return NULL;

            /* Collect args */
            size_t arg_cap = 8, arg_cnt = 0;
            cco_expr_t **args = (cco_expr_t**)malloc(arg_cap * sizeof(cco_expr_t*));
            if (!args) { p->error = 1; return NULL; }

            if (peek(p).kind != CCO_TK_RPAREN) {
                for (;;) {
                    if (arg_cnt >= arg_cap) {
                        arg_cap *= 2;
                        cco_expr_t **new_a = (cco_expr_t**)realloc(args, arg_cap * sizeof(cco_expr_t*));
                        if (!new_a) { free(args); p->error = 1; return NULL; }
                        args = new_a;
                    }
                    /* Evaluate arg value immediately for value-layer parsing */
                    cco_object_t *av = parse_value(p);
                    if (!av || p->error) { free(args); cco_object_free(av); return NULL; }
                    args[arg_cnt] = cco_expr_literal(av);
                    if (!args[arg_cnt]) { free(args); cco_object_free(av); return NULL; }
                    arg_cnt++;

                    if (match(p, CCO_TK_RPAREN)) break;
                    if (!match(p, CCO_TK_COMMA)) {
                        if (peek(p).kind == CCO_TK_RPAREN) { next(p); break; }
                        p->error = 1; free(args); return NULL;
                    }
                    if (peek(p).kind == CCO_TK_RPAREN) { next(p); break; }
                }
            } else { next(p); }

            cco_expr_t *expr = cco_expr_static_call(tname, mname, args, arg_cnt);
            free(args);
            if (!expr) { p->error = 1; return NULL; }
            cco_object_t *result = cco_expr_eval(expr, p->symtab, NULL, NULL);
            cco_expr_free(expr);
            return result;
        }

        /* #Name(...) — could be positional or named instantiation */
        if (!expect(p, CCO_TK_LPAREN)) return NULL;

        /* Check for named instantiation: .field: value */
        if (peek(p).kind == CCO_TK_DOT) {
            next(p); /* consume . */
            cco_field_init_list_t *named = cco_field_init_list_new();
            if (!named) { p->error = 1; return NULL; }

            if (peek(p).kind != CCO_TK_RPAREN) {
                for (;;) {
                    cco_token_t fname_tok = next(p);
                    if (fname_tok.kind != CCO_TK_IDENT) {
                        p->error = 1;
                        cco_field_init_list_free(named);
                        LEX_ERR(&p->lexer, "expected field name after '.'");
                        return NULL;
                    }
                    char *fn = extract_token_text(&fname_tok, p->arena);
                    if (!expect(p, CCO_TK_COLON)) { p->error = 1; cco_field_init_list_free(named); return NULL; }
                    cco_object_t *fv = parse_value(p);
                    if (!fv || p->error) { p->error = 1; cco_field_init_list_free(named); cco_object_free(fv); return NULL; }
                    cco_expr_t *fve = cco_expr_literal(fv);
                    if (!fve) { p->error = 1; cco_field_init_list_free(named); cco_object_free(fv); return NULL; }
                    cco_field_init_list_add(named, fn, fve);

                    if (match(p, CCO_TK_RPAREN)) break;
                    if (!match(p, CCO_TK_COMMA)) {
                        if (peek(p).kind == CCO_TK_RPAREN) { next(p); break; }
                        p->error = 1; cco_field_init_list_free(named); return NULL;
                    }
                    if (peek(p).kind == CCO_TK_DOT) { next(p); continue; }
                    if (peek(p).kind == CCO_TK_RPAREN) { next(p); break; }
                }
            } else { next(p); }

            cco_object_t *result = NULL;
            cco_expr_t *expr = cco_expr_instantiate(tname, NULL, 0, named);
            if (expr) {
                result = cco_expr_eval(expr, p->symtab, NULL, NULL);
                cco_expr_free(expr);
            } else { p->error = 1; }
            return result;
        }

        /* Positional instantiation: #Name(arg1, arg2, ...) */
        size_t arg_cap = 8, arg_cnt = 0;
        cco_expr_t **args = (cco_expr_t**)malloc(arg_cap * sizeof(cco_expr_t*));
        if (!args) { p->error = 1; return NULL; }

        if (peek(p).kind != CCO_TK_RPAREN) {
            for (;;) {
                if (arg_cnt >= arg_cap) {
                    arg_cap *= 2;
                    cco_expr_t **new_a = (cco_expr_t**)realloc(args, arg_cap * sizeof(cco_expr_t*));
                    if (!new_a) { free(args); p->error = 1; return NULL; }
                    args = new_a;
                }
                cco_object_t *av = parse_value(p);
                if (!av || p->error) { free(args); cco_object_free(av); return NULL; }
                args[arg_cnt] = cco_expr_literal(av);
                if (!args[arg_cnt]) { free(args); cco_object_free(av); return NULL; }
                arg_cnt++;

                if (match(p, CCO_TK_RPAREN)) break;
                if (!match(p, CCO_TK_COMMA)) {
                    if (peek(p).kind == CCO_TK_RPAREN) { next(p); break; }
                    p->error = 1; free(args); return NULL;
                }
                if (peek(p).kind == CCO_TK_RPAREN) { next(p); break; }
            }
        } else { next(p); }

        cco_expr_t *expr = cco_expr_instantiate(tname, args, arg_cnt, NULL);
        free(args);
        if (!expr) { p->error = 1; return NULL; }
        cco_object_t *result = cco_expr_eval(expr, p->symtab, NULL, NULL);
        cco_expr_free(expr);
        return result;
    }

    /* Identifier — could be: enum constant, dot-notation static call, or error */
    t = next(p);
    if (t.kind == CCO_TK_IDENT) {
        char *ident = extract_token_text(&t, p->arena);

        /* Check for dot-notation static call: Type.method(args) */
        if (peek(p).kind == CCO_TK_DOT) {
            next(p); /* consume . */
            cco_token_t mt = next(p);
            if (mt.kind == CCO_TK_IDENT && peek(p).kind == CCO_TK_LPAREN) {
                char *mname = extract_token_text(&mt, p->arena);
                next(p); /* consume ( */
                size_t ac = 0, acap = 8;
                cco_expr_t **a = (cco_expr_t**)malloc(acap * sizeof(cco_expr_t*));
                if (peek(p).kind != CCO_TK_RPAREN) {
                    for (;;) {
                        if (ac >= acap) { acap *= 2; a = (cco_expr_t**)realloc(a, acap * sizeof(cco_expr_t*)); }
                        cco_object_t *av = parse_value(p);
                        if (!av || p->error) { for (size_t k = 0; k < ac; k++) cco_expr_free(a[k]); free(a); cco_object_free(av); return NULL; }
                        a[ac] = cco_expr_literal(av); ac++;
                        if (match(p, CCO_TK_RPAREN)) break;
                        if (!match(p, CCO_TK_COMMA)) { if (peek(p).kind == CCO_TK_RPAREN) { next(p); break; } p->error = 1; for (size_t k = 0; k < ac; k++) cco_expr_free(a[k]); free(a); return NULL; }
                        if (peek(p).kind == CCO_TK_RPAREN) { next(p); break; }
                    }
                } else next(p);
                cco_expr_t *call = cco_expr_static_call(ident, mname, a, ac);
                free(a);
                if (!call) { p->error = 1; return NULL; }
                cco_object_t *res = cco_expr_eval(call, p->symtab, NULL, NULL);
                cco_expr_free(call);
                return res;
            }
            /* Not a method call — treat as error */
            p->error = 1;
            LEX_ERR(&p->lexer, "unexpected '.' after '%s'", ident);
            return NULL;
        }

        /* Check for enum constant */
        if (p->symtab) {
            size_t n = cco_symtab_get_count(p->symtab);
            for (size_t i = 0; i < n; i++) {
                const char *sym_name = cco_symtab_get_name(p->symtab, i);
                if (!sym_name) continue;
                const char **vals; size_t vc;
                if (cco_symtab_get_enum_values(p->symtab, sym_name, &vals, &vc)) {
                    for (size_t j = 0; j < vc; j++) {
                        if (strcmp(vals[j], ident) == 0)
                            return cco_int_new((long long)j);
                    }
                }
            }
        }

        /* Not an enum constant — error */
        p->error = 1;
        LEX_ERR(&p->lexer, "unexpected identifier '%s'", ident);
        return NULL;
    }
    return parse_literal(p, t);
}

/*============================================================================
 * Public API
 *============================================================================*/

cco_object_t* cco_parse(const char *text) {
    cco_diag_clear();
    cco_diag_set_source(text);
    if (!text) { CCO_ERR("cco_parse: NULL input"); return NULL; }

    cco_arena_t arena;
    cco_arena_init(&arena, 0);

    cco_parser_t parser;
    parser_init(&parser, text, "<input>", &arena);

    /* Skip leading whitespace/comments */
    cco_token_t first = peek(&parser);

    /* Check for top-level shorthand: IDENTIFIER ':' Value */
    if (first.kind == CCO_TK_IDENT) {
        cco_token_t id_tok = next(&parser);
        if (peek(&parser).kind == CCO_TK_COLON) {
            /* Single-field object shorthand */
            next(&parser); /* consume ':' */
            cco_object_t *val = parse_value(&parser);
            if (parser.error || !val) {
                cco_object_free(val);
                cco_arena_free(&arena);
                return NULL;
            }
            cco_object_t *obj = cco_object_new();
            char *key = extract_token_text(&id_tok, &arena);
            cco_object_set(obj, key, val);
            cco_arena_free(&arena);
            return obj;
        } else {
            /* Not a shorthand - it's just a literal identifier (error in value-only mode) */
            parser.error = 1;
            LEX_ERR(&parser.lexer, "unexpected identifier at line %zu col %zu",
                           id_tok.line, id_tok.col);
            cco_arena_free(&arena);
            return NULL;
        }
    }

    /* Normal value parsing */
    cco_object_t *root = parse_value(&parser);
    if (parser.error || !root) {
        cco_object_free(root);
        cco_arena_free(&arena);
        return NULL;
    }

    cco_arena_free(&arena);
    return root;
}

cco_object_t* cco_parse_from_file(const char *filename) {
    if (!filename) { CCO_ERR("cco_parse_from_file: NULL filename"); return NULL; }
    FILE *fp;
#ifdef _MSC_VER
    fopen_s(&fp, filename, "rb");
#else
    fp = fopen(filename, "rb");
#endif
    if (!fp) { CCO_ERR("cco_parse_from_file: cannot open '%s'", filename); return NULL; }
    cco_object_t *obj = cco_parse_from_stream(fp);
    fclose(fp);
    return obj;
}

cco_object_t* cco_parse_from_stream(FILE *fp) {
    if (!fp) { CCO_ERR("cco_parse_from_stream: NULL stream"); return NULL; }
    /* Read entire file */
    fseek(fp, 0, SEEK_END);
    long len = ftell(fp);
    if (len < 0) { CCO_ERR("cco_parse_from_stream: cannot determine size"); return NULL; }
    fseek(fp, 0, SEEK_SET);
    char *buf = (char*)malloc((size_t)len + 1);
    if (!buf) { CCO_ERR("cco_parse_from_stream: out of memory"); return NULL; }
    size_t read = fread(buf, 1, (size_t)len, fp);
    buf[read] = '\0';
    cco_object_t *obj = cco_parse(buf);
    free(buf);
    return obj;
}

/*============================================================================
 * Parse result
 *============================================================================*/

struct cco_parse_result {
    cco_symbol_table_t *symbols;
    cco_object_t       *root;
    cco_arena_t         arena;
};

/*============================================================================
 * Full parser (declarations + root value)
 *============================================================================*/

static int parse_typedef(cco_parser_t *p, cco_symbol_table_t *st) {
    /* $typedef . Name : TypeExpr , */
    cco_token_t t = next(p);
    if (t.kind != CCO_TK_DOT) { p->error = 1; return LEX_ERR(&p->lexer, "expected '.' after $typedef"); }
    t = next(p);
    if (t.kind != CCO_TK_IDENT) { p->error = 1; return LEX_ERR(&p->lexer, "expected type alias name"); }
    char *name = extract_token_text(&t, p->arena);
    if (!name) { p->error = 1; return -1; }

    if (!expect(p, CCO_TK_COLON)) return -1;

    /* Parse type expression */
    cco_token_t type_tok = next(p);
    cco_type_expr_t *type_expr = NULL;

    switch (type_tok.kind) {
    case CCO_TK_STRING_TYPE:
        type_expr = cco_type_primitive(CCO_PRIM_STRING);
        break;
    case CCO_TK_INTEGER_TYPE:
        type_expr = cco_type_primitive(CCO_PRIM_INTEGER);
        break;
    case CCO_TK_FLOAT_TYPE:
        type_expr = cco_type_primitive(CCO_PRIM_FLOAT);
        break;
    case CCO_TK_BOOLEAN_TYPE:
        type_expr = cco_type_primitive(CCO_PRIM_BOOLEAN);
        break;
    case CCO_TK_NONE:
        type_expr = cco_type_primitive(CCO_PRIM_NONE);
        break;
    case CCO_TK_IDENT: {
        char *s = extract_token_text(&type_tok, p->arena);
        type_expr = cco_type_named(s);
        break;
    }
    default:
        p->error = 1;
        LEX_ERR(&p->lexer, "expected type expression after ':' in $typedef");
        return -1;
    }

    if (!type_expr) { p->error = 1; return -1; }

    if (cco_symbol_table_add_typedef(st, name, type_expr) != 0) {
        p->error = 1;
        return -1;
    }
    return 0;
}

static int parse_enum(cco_parser_t *p, cco_symbol_table_t *st) {
    /* $enum . Name = ( VAL1 , VAL2 , ... ) , */
    cco_token_t t = next(p);
    if (t.kind != CCO_TK_DOT) { p->error = 1; return LEX_ERR(&p->lexer, "expected '.' after $enum"); }
    t = next(p);
    if (t.kind != CCO_TK_IDENT) { p->error = 1; return LEX_ERR(&p->lexer, "expected enum name"); }
    char *name = extract_token_text(&t, p->arena);
    if (!name) { p->error = 1; return -1; }

    if (!expect(p, CCO_TK_EQUAL)) return -1;
    if (!expect(p, CCO_TK_LPAREN)) return -1;

    /* Collect enum values (comma-separated identifiers, terminated by ')') */
    size_t val_cap = 8;
    size_t val_cnt = 0;
    const char **values = (const char**)malloc(val_cap * sizeof(const char*));
    if (!values) { p->error = 1; return -1; }

    if (peek(p).kind != CCO_TK_RPAREN) {
        for (;;) {
            t = next(p);
            if (t.kind != CCO_TK_IDENT) {
                p->error = 1;
                LEX_ERR(&p->lexer, "expected enum value identifier");
                for (size_t i = 0; i < val_cnt; i++) free((void*)values[i]);
                free(values);
                return -1;
            }
            if (val_cnt >= val_cap) {
                val_cap *= 2;
                const char **new_v = (const char**)realloc(values, val_cap * sizeof(const char*));
                if (!new_v) { p->error = 1; free(values); return -1; }
                values = new_v;
            }
            values[val_cnt] = extract_token_text(&t, p->arena);
            if (!values[val_cnt]) { p->error = 1; free(values); return -1; }
            val_cnt++;

            if (match(p, CCO_TK_RPAREN)) break;
            if (!match(p, CCO_TK_COMMA)) {
                if (peek(p).kind == CCO_TK_RPAREN) { next(p); break; }
                p->error = 1;
                LEX_ERR(&p->lexer, "expected ',' or ')' in enum");
                for (size_t i = 0; i < val_cnt; i++) free((void*)values[i]);
                free(values);
                return -1;
            }
            if (peek(p).kind == CCO_TK_RPAREN) { next(p); break; }
        }
    } else {
        next(p); /* consume ')' */
    }

    /* Build null-terminated array for the symbol table function */
    const char **null_term = (const char**)malloc((val_cnt + 1) * sizeof(const char*));
    if (!null_term) { p->error = 1; free(values); return -1; }
    for (size_t i = 0; i < val_cnt; i++)
        null_term[i] = values[i];
    null_term[val_cnt] = NULL;

    int ret = cco_symbol_table_add_enum(st, name, null_term);
    free(null_term);
    free(values);
    if (ret != 0) { p->error = 1; return -1; }
    return 0;
}

static int parse_top_item(cco_parser_t *p, cco_symbol_table_t *st, cco_object_t **root_out) {
    cco_token_t t = peek(p);

    if (t.kind == CCO_TK_TYPEDEF) {
        next(p); /* consume $typedef */
        return parse_typedef(p, st);
    }
    if (t.kind == CCO_TK_ENUM) {
        next(p); /* consume $enum */
        return parse_enum(p, st);
    }
    if (t.kind == CCO_TK_TEMP) {
        next(p); /* consume $temp */

        cco_token_t dot = next(p);
        if (dot.kind != CCO_TK_DOT) { p->error = 1; return LEX_ERR(&p->lexer, "expected '.' after $temp"); }

        /* Optional generic params: $temp.<T>Name */
        if (peek(p).kind == CCO_TK_LT) {
            int depth = 1; next(p);
            while (depth > 0 && peek(p).kind != CCO_TK_EOF) {
                cco_token_t gt = next(p);
                if (gt.kind == CCO_TK_LT) depth++;
                else if (gt.kind == CCO_TK_GT) depth--;
            }
        }

        cco_token_t name_tok = next(p);
        if (name_tok.kind != CCO_TK_IDENT) { p->error = 1; return LEX_ERR(&p->lexer, "expected template name"); }
        char *tname = extract_token_text(&name_tok, p->arena);

        /* Optional parent: + ParentName */
        char *parent_name = NULL;
        if (peek(p).kind == CCO_TK_PLUS) {
            next(p);
            cco_token_t parent_tok = next(p);
            if (parent_tok.kind != CCO_TK_IDENT) { p->error = 1;
                return LEX_ERR(&p->lexer, "expected parent template name after '+'"); }
            parent_name = extract_token_text(&parent_tok, p->arena);
        }

        if (!expect(p, CCO_TK_COLON)) return -1;
        if (!expect(p, CCO_TK_LPAREN)) return -1;

        cco_template_t *tmpl = cco_template_new(tname, parent_name);
        if (!tmpl) { p->error = 1; return -1; }

        /* Parse template body items (fields, constructors, static methods) */
        if (peek(p).kind != CCO_TK_RPAREN) {
            for (;;) {
                cco_token_t item_start = peek(p);
                if (item_start.kind == CCO_TK_FUNCTION) {
                    next(p);
                    if (peek(p).kind == CCO_TK_DOT) {
                        next(p);
                        cco_token_t func_type = next(p);
                        if (func_type.kind == CCO_TK_AT) {
                            /* Constructor: $function.@ <params> : ( body ) or $function.@ : $default */
                            cco_constructor_t *ctor = NULL;

                            if (peek(p).kind == CCO_TK_COLON) {
                                next(p);
                                if (peek(p).kind == CCO_TK_DEFAULT) {
                                    next(p);
                                    ctor = cco_constructor_default();
                                } else {
                                    p->error = 1;
                                    LEX_ERR(&p->lexer, "expected $default after '$function.@:'");
                                    cco_template_free(tmpl);
                                    return -1;
                                }
                            } else if (peek(p).kind == CCO_TK_LT) {
                                /* $function.@<params>: ( body ) */
                                next(p); /* consume < */
                                /* Parse params: name: Type, ... */
                                size_t pc = 0, pcap = 8;
                                const char **pnames = (const char**)malloc(pcap * sizeof(const char*));
                                cco_type_expr_t **ptypes = (cco_type_expr_t**)calloc(pcap, sizeof(cco_type_expr_t*));
                                if (!pnames || !ptypes) { free(pnames); free(ptypes); p->error = 1; return -1; }

                                while (peek(p).kind != CCO_TK_GT) {
                                    if (pc >= pcap) {
                                        pcap *= 2;
                                        pnames = (const char**)realloc(pnames, pcap * sizeof(const char*));
                                        ptypes = (cco_type_expr_t**)realloc(ptypes, pcap * sizeof(cco_type_expr_t*));
                                        if (!pnames || !ptypes) { free(pnames); free(ptypes); p->error = 1; return -1; }
                                    }
                                    cco_token_t pn = next(p);
                                    if (pn.kind != CCO_TK_IDENT && pn.kind != CCO_TK_ANON_PARAM) {
                                        p->error = 1; free(pnames); free(ptypes);
                                        LEX_ERR(&p->lexer, "expected parameter name"); return -1;
                                    }
                                    pnames[pc] = extract_token_text(&pn, p->arena);
                                    if (!expect(p, CCO_TK_COLON)) { free(pnames); free(ptypes); return -1; }
                                    cco_token_t pt = next(p);
                                    if (pt.kind == CCO_TK_INTEGER_TYPE)
                                        ptypes[pc] = cco_type_primitive(CCO_PRIM_INTEGER);
                                    else if (pt.kind == CCO_TK_STRING_TYPE)
                                        ptypes[pc] = cco_type_primitive(CCO_PRIM_STRING);
                                    else if (pt.kind == CCO_TK_FLOAT_TYPE)
                                        ptypes[pc] = cco_type_primitive(CCO_PRIM_FLOAT);
                                    else if (pt.kind == CCO_TK_BOOLEAN_TYPE)
                                        ptypes[pc] = cco_type_primitive(CCO_PRIM_BOOLEAN);
                                    else if (pt.kind == CCO_TK_IDENT) {
                                        char *tn = extract_token_text(&pt, p->arena);
                                        ptypes[pc] = cco_type_named(tn);
                                    }
                                    pc++;

                                    if (peek(p).kind == CCO_TK_COMMA) next(p);
                                }
                                next(p); /* consume > */

                                if (!expect(p, CCO_TK_COLON)) { free(pnames); free(ptypes); return -1; }
                                /* Parse body: ( ... ) */
                                if (!expect(p, CCO_TK_LPAREN)) { free(pnames); free(ptypes); return -1; }

                                /* Parse constructor body: $this.(field: expr, ...) */
                                cco_method_body_t *body = cco_method_body_new();

                                while (peek(p).kind != CCO_TK_RPAREN && peek(p).kind != CCO_TK_EOF) {
                                    cco_token_t bt = next(p);
                                    if (bt.kind == CCO_TK_THIS_REF || bt.kind == CCO_TK_THIS) {
                                        if (peek(p).kind == CCO_TK_DOT) {
                                            next(p); /* consume . */
                                            if (peek(p).kind == CCO_TK_LPAREN) {
                                                next(p); /* consume ( */
                                                while (peek(p).kind != CCO_TK_RPAREN && peek(p).kind != CCO_TK_EOF) {
                                                    cco_token_t fn = next(p);
                                                    if (fn.kind == CCO_TK_IDENT) {
                                                        char *init_name = extract_token_text(&fn, p->arena);
                                                        if (!expect(p, CCO_TK_COLON)) break;

                                                        /* Parse the value expression: could be literal, identifier, or operator expr */
                                                        cco_token_t vt = peek(p);
                                                        cco_expr_t *ie = NULL;

                                                        if (vt.kind == CCO_TK_IDENT) {
                                                            /* Identifier reference (parameter name) */
                                                            cco_token_t id_tok = next(p);
                                                            char *id_name = extract_token_text(&id_tok, p->arena);
                                                            ie = cco_expr_identifier(id_name);

                                                            /* Check for operator: e.g., mana * 2 */
                                                            if (peek(p).kind == CCO_TK_STAR || peek(p).kind == CCO_TK_PLUS ||
                                                                peek(p).kind == CCO_TK_MINUS || peek(p).kind == CCO_TK_SLASH) {
                                                                cco_op_t op;
                                                                cco_token_t op_tok = next(p);
                                                                if (op_tok.kind == CCO_TK_PLUS) op = CCO_OP_ADD;
                                                                else if (op_tok.kind == CCO_TK_MINUS) op = CCO_OP_SUB;
                                                                else if (op_tok.kind == CCO_TK_STAR) op = CCO_OP_MUL;
                                                                else op = CCO_OP_DIV;
                                                                cco_token_t rt = next(p);
                                                                if (rt.kind == CCO_TK_INT_LIT) {
                                                                    char buf[64]; size_t j = 0;
                                                                    for (size_t k = 0; k < rt.length && j < 63; k++)
                                                                        if (rt.start[k] != '_') buf[j++] = rt.start[k];
                                                                    buf[j] = '\0';
                                                                    ie = cco_expr_binary(ie, op, cco_expr_literal(cco_int_new(strtoll(buf, NULL, 0))));
                                                                } else if (rt.kind == CCO_TK_FLOAT_LIT) {
                                                                    char buf[64]; size_t j = 0;
                                                                    for (size_t k = 0; k < rt.length && j < 63; k++)
                                                                        if (rt.start[k] != '_') buf[j++] = rt.start[k];
                                                                    buf[j] = '\0';
                                                                    ie = cco_expr_binary(ie, op, cco_expr_literal(cco_float_new(strtod(buf, NULL))));
                                                                } else if (rt.kind == CCO_TK_IDENT) {
                                                                    char *rn = extract_token_text(&rt, p->arena);
                                                                    ie = cco_expr_binary(ie, op, cco_expr_identifier(rn));
                                                                }
                                                            }
                                                        } else {
                                                            cco_object_t *iv = parse_value(p);
                                                            if (iv) ie = cco_expr_literal(iv);
                                                        }

                                                        if (ie) {
                                                            cco_stmt_t *as = cco_stmt_assign(init_name, ie);
                                                            cco_method_body_add_stmt(body, as);
                                                        }
                                                    } else if (fn.kind == CCO_TK_COMMA || fn.kind == CCO_TK_RPAREN) {
                                                        if (fn.kind == CCO_TK_RPAREN) break;
                                                    }
                                                }
                                                if (peek(p).kind == CCO_TK_RPAREN) next(p);
                                            }
                                        }
                                    } else if (bt.kind == CCO_TK_COMMA) {
                                        continue;
                                    }
                                }
                                if (peek(p).kind == CCO_TK_RPAREN) next(p);

                                cco_param_list_t *pl = cco_param_list_new(pc, pnames, ptypes);
                                free(pnames); free(ptypes);
                                ctor = cco_constructor_new(pl, body);
                            } else if (peek(p).kind == CCO_TK_COLON) {
                                next(p);
                                if (!expect(p, CCO_TK_LPAREN)) { cco_template_free(tmpl); return -1; }
                                while (peek(p).kind != CCO_TK_RPAREN && peek(p).kind != CCO_TK_EOF)
                                    next(p);
                                if (peek(p).kind == CCO_TK_RPAREN) next(p);
                                ctor = cco_constructor_new(NULL, NULL);
                            }

                            if (ctor) cco_template_add_constructor(tmpl, ctor);
                        } else if (func_type.kind == CCO_TK_HASH || func_type.kind == CCO_TK_LT) {
                            /* Static method: $function.#name or $function.<generic>!#name */
                            cco_type_expr_t **gtypes = NULL;
                            size_t gc = 0;
                            int has_priv = 0;
                            char *sm_name = NULL;

                            if (func_type.kind == CCO_TK_LT) {
                                /* <generic>
                                   Simply.cco has: <T: dyn Character>
                                   We'll skip generic parsing for now */
                                int depth = 1;
                                while (depth > 0 && peek(p).kind != CCO_TK_EOF) {
                                    cco_token_t gt = next(p);
                                    if (gt.kind == CCO_TK_LT) depth++;
                                    else if (gt.kind == CCO_TK_GT) depth--;
                                }
                                func_type = next(p); /* should be ! or # */
                            }
                            if (func_type.kind == CCO_TK_NOT) {
                                has_priv = 1;
                                func_type = next(p);
                            }
                            if (func_type.kind == CCO_TK_HASH) {
                                cco_token_t mn = next(p);
                                if (mn.kind == CCO_TK_IDENT) {
                                    sm_name = extract_token_text(&mn, p->arena);
                                }
                            }
                            if (sm_name) {
                                /* Skip rest of method signature and body:
                                   #methodName<generic?>(params) : ReturnType ( body ) */
                                /* Skip <generic> */
                                if (peek(p).kind == CCO_TK_LT) {
                                    int d = 1; next(p);
                                    while (d > 0 && peek(p).kind != CCO_TK_EOF) {
                                        cco_token_t gt = next(p);
                                        if (gt.kind == CCO_TK_LT) d++;
                                        else if (gt.kind == CCO_TK_GT) d--;
                                    }
                                }
                                /* Skip (params) */
                                if (peek(p).kind == CCO_TK_LPAREN) {
                                    int d2 = 1; next(p);
                                    while (d2 > 0 && peek(p).kind != CCO_TK_EOF) {
                                        cco_token_t ct = next(p);
                                        if (ct.kind == CCO_TK_LPAREN) d2++;
                                        else if (ct.kind == CCO_TK_RPAREN) d2--;
                                    }
                                }
                                /* Skip : ReturnType (body) */
                                if (peek(p).kind == CCO_TK_COLON) {
                                    next(p);
                                    /* Skip return type identifier (e.g., T) */
                                    if (peek(p).kind == CCO_TK_IDENT || peek(p).kind == CCO_TK_INTEGER_TYPE ||
                                        peek(p).kind == CCO_TK_STRING_TYPE || peek(p).kind == CCO_TK_FLOAT_TYPE ||
                                        peek(p).kind == CCO_TK_BOOLEAN_TYPE || peek(p).kind == CCO_TK_NONE)
                                        next(p);
                                    /* Skip body ( ... ) */
                                    if (peek(p).kind == CCO_TK_LPAREN) {
                                        int d3 = 1; next(p);
                                        while (d3 > 0 && peek(p).kind != CCO_TK_EOF) {
                                            cco_token_t bt = next(p);
                                            if (bt.kind == CCO_TK_LPAREN) d3++;
                                            else if (bt.kind == CCO_TK_RPAREN) d3--;
                                        }
                                    }
                                }
                                /* Create a stub static method */
                                cco_static_method_t *sm = cco_static_method_new(
                                    sm_name, has_priv, NULL, 0, NULL, NULL, NULL);
                                cco_template_add_static_method(tmpl, sm);
                            }
                            free(gtypes);
                        }
                    }
                } else if (item_start.kind == CCO_TK_IDENT) {
                    /* Field declaration: name : Type [= default] */
                    cco_token_t fname_tok = next(p);
                    char *fname = extract_token_text(&fname_tok, p->arena);

                    /* Optional type annotation: <TypeExpr> */
                    cco_type_expr_t *ftype = NULL;
                    cco_expr_t *fdefault = NULL;

                    if (peek(p).kind == CCO_TK_LT) {
                        /* Skip < TypeExpr > with nested <> support */
                        int depth = 1; next(p);
                        while (depth > 0 && peek(p).kind != CCO_TK_EOF) {
                            cco_token_t at = next(p);
                            if (at.kind == CCO_TK_LT) depth++;
                            else if (at.kind == CCO_TK_GT) depth--;
                        }
                    }

                    if (!expect(p, CCO_TK_COLON)) { cco_type_expr_free(ftype); return -1; }

                    /* Type expr after ':' */
                    cco_token_t type_token = next(p);
                    cco_type_expr_t *field_type = NULL;
                    switch (type_token.kind) {
                    case CCO_TK_STRING_TYPE:    field_type = cco_type_primitive(CCO_PRIM_STRING); break;
                    case CCO_TK_INTEGER_TYPE:   field_type = cco_type_primitive(CCO_PRIM_INTEGER); break;
                    case CCO_TK_FLOAT_TYPE:     field_type = cco_type_primitive(CCO_PRIM_FLOAT); break;
                    case CCO_TK_BOOLEAN_TYPE:   field_type = cco_type_primitive(CCO_PRIM_BOOLEAN); break;
                    case CCO_TK_NONE:           field_type = cco_type_primitive(CCO_PRIM_NONE); break;
                    case CCO_TK_IDENT: {
                        char *tn = extract_token_text(&type_token, p->arena);
                        field_type = cco_type_named(tn);
                        break;
                    }
                    default:
                        p->error = 1;
                        LEX_ERR(&p->lexer, "expected type in field declaration");
                        cco_type_expr_free(ftype);
                        return -1;
                    }
                    if (ftype) cco_type_expr_free(ftype); /* annotation advisory only */

                    /* Optional default value: = expr */
                    if (peek(p).kind == CCO_TK_EQUAL) {
                        next(p);
                        cco_object_t *dv = parse_value(p);
                        if (dv) fdefault = cco_expr_literal(dv);
                        if (!fdefault) { cco_type_expr_free(field_type); p->error = 1; return -1; }
                    }

                    cco_template_add_field(tmpl, fname, field_type, fdefault);
                } else {
                    p->error = 1;
                    LEX_ERR(&p->lexer, "unexpected token in template body");
                    cco_template_free(tmpl);
                    return -1;
                }

                if (peek(p).kind == CCO_TK_RPAREN) { next(p); break; }
                if (!match(p, CCO_TK_COMMA)) {
                    if (peek(p).kind == CCO_TK_RPAREN) { next(p); break; }
                    p->error = 1;
                    LEX_ERR(&p->lexer, "expected ',' or ')' in template body");
                    cco_template_free(tmpl);
                    return -1;
                }
                if (peek(p).kind == CCO_TK_RPAREN) { next(p); break; }
            }
        } else {
            next(p); /* consume ')' */
        }

        /* Add default constructor automatically */
        cco_template_add_constructor(tmpl, cco_constructor_default());

        if (cco_symbol_table_add_template(st, tmpl) != 0) {
            p->error = 1;
            cco_template_free(tmpl);
            return -1;
        }
        return 0;
    }

    /* Root value expression */
    cco_object_t *root = NULL;

    /* Check for top-level shorthand */
    if (t.kind == CCO_TK_IDENT) {
        cco_token_t id_tok = next(p);
        if (peek(p).kind == CCO_TK_COLON) {
            next(p);
            cco_object_t *val = parse_value(p);
            if (p->error || !val) { cco_object_free(val); return -1; }
            root = cco_object_new();
            char *key = extract_token_text(&id_tok, p->arena);
            cco_object_set(root, key, val);
            *root_out = root;
            return 0;
        }
        /* Not shorthand, put identifier back and try as value */
        /* We can't put back easily. Set error for now. */
        p->error = 1;
        LEX_ERR(&p->lexer, "unexpected identifier in declaration context");
        return -1;
    }

    if (t.kind == CCO_TK_LPAREN || t.kind == CCO_TK_HASH) {
        root = parse_value(p);
        if (p->error || !root) { cco_object_free(root); return -1; }
        *root_out = root;
        return 0;
    }

    /* Literal as root */
    t = next(p);
    root = parse_literal(p, t);
    if (p->error || !root) { cco_object_free(root); return -1; }
    *root_out = root;
    return 0;
}

cco_parse_result_t* cco_parse_full(const char *text) {
    cco_diag_clear();
    cco_diag_set_source(text);
    if (!text) { CCO_ERR("cco_parse_full: NULL input"); return NULL; }

    cco_parse_result_t *res = (cco_parse_result_t*)calloc(1, sizeof(cco_parse_result_t));
    if (!res) return NULL;

    cco_arena_init(&res->arena, 0);
    res->symbols = cco_symbol_table_new();
    if (!res->symbols) { cco_arena_free(&res->arena); free(res); return NULL; }

    cco_parser_t parser;
    parser_init(&parser, text, "<input>", &res->arena);
    parser.symtab = res->symbols;

    int has_root = 0;

    /* Parse top-level items separated by commas */
    for (;;) {
        cco_token_t t = peek(&parser);
        if (t.kind == CCO_TK_EOF) break;

        cco_object_t *root_val = NULL;
        int r = parse_top_item(&parser, res->symbols, &root_val);

        if (parser.error) {
            cco_parse_result_free(res);
            return NULL;
        }

        if (root_val) {
            if (has_root) {
                cco_object_free(root_val);
                CCO_ERR("multiple root expressions");
                cco_parse_result_free(res);
                return NULL;
            }
            res->root = root_val;
            has_root = 1;
        }

        /* Skip optional comma */
        if (peek(&parser).kind == CCO_TK_COMMA)
            next(&parser);
    }

    if (!has_root) {
         CCO_ERR("no root expression in .cco file");
        cco_parse_result_free(res);
        return NULL;
    }

    return res;
}

cco_parse_result_t* cco_parse_full_from_file(const char *filename) {
    if (!filename) { CCO_ERR("cco_parse_full_from_file: NULL filename"); return NULL; }
    FILE *fp;
#ifdef _MSC_VER
    fopen_s(&fp, filename, "rb");
#else
    fp = fopen(filename, "rb");
#endif
    if (!fp) { CCO_ERR("cco_parse_full_from_file: cannot open '%s'", filename); return NULL; }
    fseek(fp, 0, SEEK_END);
    long len = ftell(fp);
    fseek(fp, 0, SEEK_SET);
    char *buf = (char*)malloc((size_t)len + 1);
    if (!buf) { fclose(fp); CCO_ERR("out of memory"); return NULL; }
    size_t read_sz = fread(buf, 1, (size_t)len, fp);
    buf[read_sz] = '\0';
    fclose(fp);
    cco_parse_result_t *res = cco_parse_full(buf);
    free(buf);
    return res;
}

cco_parse_result_t* cco_parse_full_from_stream(FILE *fp) {
    if (!fp) { CCO_ERR("cco_parse_full_from_stream: NULL stream"); return NULL; }
    fseek(fp, 0, SEEK_END);
    long len = ftell(fp);
    fseek(fp, 0, SEEK_SET);
    char *buf = (char*)malloc((size_t)len + 1);
    if (!buf) { CCO_ERR("out of memory"); return NULL; }
    size_t read_sz = fread(buf, 1, (size_t)len, fp);
    buf[read_sz] = '\0';
    cco_parse_result_t *res = cco_parse_full(buf);
    free(buf);
    return res;
}

cco_symbol_table_t* cco_parse_result_symbols(cco_parse_result_t *res) {
    return res ? res->symbols : NULL;
}

cco_object_t* cco_parse_result_root(cco_parse_result_t *res) {
    return res ? res->root : NULL;
}

void cco_parse_result_free(cco_parse_result_t *res) {
    if (!res) return;
    cco_symbol_table_free(res->symbols);
    cco_object_free(res->root);
    cco_arena_free(&res->arena);
    free(res);
}
