#ifndef CNT_CCO_INTERNAL_EXPR_H
#define CNT_CCO_INTERNAL_EXPR_H

#include "parser.h"

/* Operator definitions */
typedef enum {
    CCO_OP_NONE = 0,
    CCO_OP_OR,      /* || */
    CCO_OP_AND,     /* && */
    CCO_OP_BIT_OR,  /* | */
    CCO_OP_BIT_XOR, /* ^ */
    CCO_OP_BIT_AND, /* & */
    CCO_OP_EQ,      /* == */
    CCO_OP_NEQ,     /* != */
    CCO_OP_LT,      /* < */
    CCO_OP_LTE,     /* <= */
    CCO_OP_GT,      /* > */
    CCO_OP_GTE,     /* >= */
    CCO_OP_COALESCE,/* ?? */
    CCO_OP_ADD,     /* + */
    CCO_OP_SUB,     /* - */
    CCO_OP_MUL,     /* * */
    CCO_OP_DIV,     /* / */
    CCO_OP_POW,     /* ** */
    CCO_OP_NOT,     /* ! */
    CCO_OP_BIT_NOT, /* ~ */
    CCO_OP_NEG,     /* unary - */
    CCO_OP_POS      /* unary + */
} cco_op_t;

typedef enum {
    CCO_EXPR_LITERAL,
    CCO_EXPR_IDENTIFIER,
    CCO_EXPR_UNARY,
    CCO_EXPR_BINARY,
    CCO_EXPR_TERNARY,
    CCO_EXPR_RANGE,
    CCO_EXPR_CALL
} cco_expr_type_t;

typedef struct cco_expr_s cco_expr_t;

struct cco_expr_s {
    cco_expr_type_t type;
    union {
        cco_object_t* literal;
        struct {
            char* name;
            size_t length;
        } identifier;
        struct {
            cco_op_t op;
            cco_expr_t* right;
        } unary;
        struct {
            cco_op_t op;
            cco_expr_t* left;
            cco_expr_t* right;
        } binary;
        struct {
            cco_expr_t* condition;
            cco_expr_t* true_branch;
            cco_expr_t* false_branch;
        } ternary;
        struct {
            cco_expr_t* start;
            cco_expr_t* end;
            bool inclusive;
        } range;
        struct {
            char* name;
            cco_expr_t** args;
            size_t arg_count;
        } call;
    } as;
};

/* AST creation */
cco_expr_t* cco_expr_new_literal(cco_object_t* obj);
cco_expr_t* cco_expr_new_identifier(const char* name, size_t len);
cco_expr_t* cco_expr_new_unary(cco_op_t op, cco_expr_t* right);
cco_expr_t* cco_expr_new_binary(cco_op_t op, cco_expr_t* left, cco_expr_t* right);
cco_expr_t* cco_expr_new_ternary(cco_expr_t* cond, cco_expr_t* t, cco_expr_t* f);
cco_expr_t* cco_expr_new_range(cco_expr_t* start, cco_expr_t* end, bool inclusive);
cco_expr_t* cco_expr_new_call(const char* name, size_t len);
void cco_expr_call_add_arg(cco_expr_t* call, cco_expr_t* arg);

/* Parsing & Evaluation */
cco_expr_t* cco_parse_expr(cco_parser_context_t* ctx, int min_prec);
cco_object_t* cco_eval_expr(cco_parser_context_t* ctx, cco_expr_t* expr);
void cco_expr_free(cco_expr_t* expr);

#endif /* CNT_CCO_INTERNAL_EXPR_H */
