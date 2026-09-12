#ifndef CNT_CCO_INTERNAL_EVAL_H
#define CNT_CCO_INTERNAL_EVAL_H

#include "object.h"
#include "parser.h"

/* Evaluates an expression. If evaluation is disabled via macros/options,
   returns CCO_ERR_FORBIDDEN. */
cco_error_t cco_eval_expression(cco_parser_context_t* ctx,
                                cco_object_t* expr_ast, cco_object_t** out_val);

/* Evaluates the $format builtin */
cco_error_t cco_eval_builtin_format(cco_parser_context_t* ctx,
                                    const char* format_str,
                                    cco_object_t** out_val);

/* Evaluates the $env builtin */
cco_error_t cco_eval_builtin_env(cco_parser_context_t* ctx,
                                 const char* env_name, cco_object_t** out_val);

#endif /* CNT_CCO_INTERNAL_EVAL_H */
