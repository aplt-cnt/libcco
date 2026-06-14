#ifndef CCO_PRIVATE_H
#define CCO_PRIVATE_H

#include <cnt/cco.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

/*============================================================================
 * Arena allocator
 *============================================================================*/
typedef struct cco_arena {
    char  *memory;
    size_t capacity;
    size_t offset;
} cco_arena_t;

void  cco_arena_init(cco_arena_t *a, size_t cap);
void *cco_arena_alloc(cco_arena_t *a, size_t size);
void  cco_arena_reset(cco_arena_t *a);
void  cco_arena_free(cco_arena_t *a);

/*============================================================================
 * Diagnostics (internal)
 *============================================================================*/
#define CCO_MAX_DIAGS 32

void cco_diag_clear(void);
int  cco_diag_vfail(const char *file, size_t line, size_t col, const char *fmt, ...);
int  cco_diag_vwarn(const char *file, size_t line, size_t col, const char *fmt, ...);

/* Get the original source text for context printing (set by lexer/parser) */
void cco_diag_set_source(const char *source);
const char* cco_diag_get_source(void);

/* Convenience macros for diagnostics */
#define CCO_ERR(fmt, ...)    cco_diag_vfail(NULL, 0, 0, fmt, ##__VA_ARGS__)
#define CCO_WARN(fmt, ...)   cco_diag_vwarn(NULL, 0, 0, fmt, ##__VA_ARGS__)
#define CCO_ERR_AT(f,l,c,fmt,...) cco_diag_vfail(f, l, c, fmt, ##__VA_ARGS__)

/* Parser/lexer error with location from lexer state */
#define LEX_ERR(lx, fmt, ...) \
    cco_diag_vfail((lx)->source.filename, (lx)->line, (lx)->col, fmt, ##__VA_ARGS__)

/*============================================================================
 * Source text
 *============================================================================*/
typedef struct cco_source {
    const char *data;
    const char *filename;
    size_t      len;
} cco_source_t;

void cco_source_init(cco_source_t *s, const char *text, const char *filename);

/*============================================================================
 * Token types
 *============================================================================*/
typedef enum cco_token_kind {
    CCO_TK_EOF,
    CCO_TK_ERROR,

    /* Keywords */
    CCO_TK_TRUE, CCO_TK_FALSE, CCO_TK_NONE,
    CCO_TK_STRING_TYPE, CCO_TK_INTEGER_TYPE,
    CCO_TK_FLOAT_TYPE, CCO_TK_BOOLEAN_TYPE,
    CCO_TK_ARRAY, CCO_TK_DYN, CCO_TK_THIS,

    /* $-keywords */
    CCO_TK_TYPEDEF, CCO_TK_ENUM, CCO_TK_TEMP,
    CCO_TK_FUNCTION, CCO_TK_DEFAULT,
    CCO_TK_THIS_REF, CCO_TK_RETURN, CCO_TK_FORMAT,

    /* Literals */
    CCO_TK_STRING_LIT, CCO_TK_RAW_STRING_LIT,
    CCO_TK_INT_LIT, CCO_TK_FLOAT_LIT,

    /* Identifiers */
    CCO_TK_IDENT, CCO_TK_ANON_PARAM,

    /* Delimiters / Operators */
    CCO_TK_LPAREN, CCO_TK_RPAREN,
    CCO_TK_COLON, CCO_TK_COMMA, CCO_TK_EQUAL,
    CCO_TK_PLUS, CCO_TK_MINUS, CCO_TK_STAR, CCO_TK_SLASH,
    CCO_TK_EQEQ, CCO_TK_NEQ,
    CCO_TK_LT, CCO_TK_GT, CCO_TK_LTEQ, CCO_TK_GTEQ,
    CCO_TK_AND, CCO_TK_OR, CCO_TK_NOT,
    CCO_TK_ARROW, CCO_TK_DOT, CCO_TK_HASH,
    CCO_TK_DOLLAR, CCO_TK_AT, CCO_TK_PIPE, CCO_TK_BACKTICK,
} cco_token_kind_t;

typedef struct cco_token {
    cco_token_kind_t kind;
    const char      *start;
    size_t           length;
    size_t           line;
    size_t           col;
} cco_token_t;

/*============================================================================
 * Lexer
 *============================================================================*/
typedef struct cco_lexer {
    cco_source_t   source;
    size_t         pos;
    size_t         line;
    size_t         col;
    cco_token_t    current;
    cco_token_t    peek;
    int            has_peek;
    cco_arena_t   *arena;
    int            error_flag;
} cco_lexer_t;

void       cco_lexer_init(cco_lexer_t *lx, const char *text,
                          const char *filename, cco_arena_t *arena);
cco_token_t cco_lexer_next(cco_lexer_t *lx);
cco_token_t cco_lexer_peek(cco_lexer_t *lx);
int        cco_lexer_ok(const cco_lexer_t *lx);

/*============================================================================
 * Object internal structure
 *============================================================================*/
typedef struct cco_field {
    char        *key;
    cco_object_t *value;
} cco_field_t;

struct cco_object {
    cco_value_type_t type;
    char       *template_name;   /* non-NULL only for TEMPLATE_INSTANCE */
    cco_field_t *fields;         /* ordered key-value pairs (MAP + TEMPLATE_INSTANCE) */
    size_t       field_count;
    size_t       field_capacity;
    union {
        int            bool_val;
        long long      int_val;
        double         float_val;
        struct {
            char  *data;
            size_t len;
        } string;
        cco_array_t   *array;
    } data;
};

struct cco_array {
    cco_object_t **items;
    size_t         count;
    size_t         capacity;
};

struct cco_map_iter {
    const cco_object_t *map;
    size_t              index;
};

/* Field init list internal (for evaluator access) */
struct cco_field_init_list {
    char       **field_names;
    cco_expr_t **value_exprs;
    size_t       count;
    size_t       capacity;
};

/* Internal field helpers */
int  cco_grow_fields(cco_field_t **fields, size_t *cap, size_t min);
int  cco_field_find(const cco_field_t *fields, size_t count, const char *key, size_t *out);

/* Type expression struct (for serialization/evaluation) */
struct cco_type_expr {
    cco_type_kind_t kind;
    union {
        cco_primitive_type_t primitive;
        struct { struct cco_type_expr *element_type; } array;
        struct { cco_field_type_pair_t *fields; size_t count; } object;
        struct { char *name; } named;
        struct { char *name; } dyn;
    } data;
};

/* Template name & parent query */
const char* cco_template_get_name(const cco_template_t *tmpl);
const char* cco_template_get_parent_name(const cco_template_t *tmpl);

/* Subtype check: is child_tmpl a subtype of base_tmpl (or equal)? */
int cco_template_is_subtype(cco_symbol_table_t *st, const char *child, const char *base);

/* Template field query (used by evaluator) */
size_t      cco_template_field_count(const cco_template_t *tmpl);
const char* cco_template_field_name(const cco_template_t *tmpl, size_t idx);
cco_expr_t* cco_template_field_default(const cco_template_t *tmpl, size_t idx);

/* Template constructor query (used by evaluator) */
size_t            cco_template_constructor_count(const cco_template_t *tmpl);
cco_constructor_t* cco_template_get_constructor(const cco_template_t *tmpl, size_t idx);

/* Constructor/param query */
int          cco_constructor_is_default(const cco_constructor_t *ctor);
size_t       cco_constructor_param_count(const cco_constructor_t *ctor);
const char*  cco_constructor_param_name(const cco_constructor_t *ctor, size_t idx);
cco_method_body_t* cco_constructor_get_body(const cco_constructor_t *ctor);

/* Symbol table: add template by name */
int cco_symtab_add_template_named(cco_symbol_table_t *st, const char *name, cco_template_t *tmpl);

/* Symbol table iteration (for serialization) */
size_t cco_symtab_get_count(const cco_symbol_table_t *st);
const char* cco_symtab_get_name(const cco_symbol_table_t *st, size_t idx);
cco_type_expr_t* cco_symtab_get_typedef(const cco_symbol_table_t *st, const char *name);
int cco_symtab_get_enum_values(const cco_symbol_table_t *st, const char *name, const char ***out_values, size_t *out_count);
cco_template_t* cco_symtab_get_template(const cco_symbol_table_t *st, const char *name);

/* Method body & statement internals (for evaluator) */
struct cco_method_body {
    cco_stmt_t **stmts;
    size_t       stmt_count;
    size_t       stmt_capacity;
    cco_expr_t  *final_expr;
};

struct cco_stmt {
    cco_stmt_kind_t kind;
    union {
        struct { char *name; cco_type_expr_t *type; cco_expr_t *value; } bind;
        struct { char *field_name; cco_expr_t *value; } assign;
        struct { cco_type_expr_t *type; cco_expr_t *value; } return_stmt;
        struct { cco_expr_t *expr; } expr_stmt;
    } data;
};

#endif /* CCO_PRIVATE_H */
