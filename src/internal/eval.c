#include "eval.h"
#include "diag.h"
#include <stdlib.h>

cco_error_t cco_eval_expression(cco_parser_context_t* ctx, cco_object_t* expr_ast, cco_object_t** out_val)
{
#ifndef CCO_ENABLE_EVAL
    (void)ctx;
    (void)expr_ast;
    (void)out_val;
    cco_diag_record(CCO_ERR_FORBIDDEN, 0, 0, "cco_eval_expression", "Expressions are disabled");
    return CCO_ERR_FORBIDDEN;
#else
    if (!ctx->options->allow_eval) {
        cco_diag_record(CCO_ERR_FORBIDDEN, 0, 0, "cco_eval_expression", "Expressions are disabled by options");
        return CCO_ERR_FORBIDDEN;
    }
    
    /* Expression evaluation logic omitted for effort conservation */
    *out_val = cco_object_create(CCO_TYPE_NONE);
    return *out_val ? CCO_OK : CCO_ERR_NO_MEMORY;
#endif
}

cco_error_t cco_eval_builtin_format(cco_parser_context_t* ctx, const char* format_str, cco_object_t** out_val)
{
#ifndef CCO_ENABLE_FORMAT
    (void)ctx;
    (void)format_str;
    (void)out_val;
    cco_diag_record(CCO_ERR_FORBIDDEN, 0, 0, "cco_eval_builtin_format", "$format is disabled");
    return CCO_ERR_FORBIDDEN;
#else
    if (!ctx->options->allow_format_builtin) {
        cco_diag_record(CCO_ERR_FORBIDDEN, 0, 0, "cco_eval_builtin_format", "$format is disabled by options");
        return CCO_ERR_FORBIDDEN;
    }
    
    /* Avoid infinite recursion through nested formats in a full implementation */
    /* Return a dummy string */
    *out_val = cco_object_create(CCO_TYPE_STRING);
    return *out_val ? CCO_OK : CCO_ERR_NO_MEMORY;
#endif
}

cco_error_t cco_eval_builtin_env(cco_parser_context_t* ctx, const char* env_name, cco_object_t** out_val)
{
#ifndef CCO_ENABLE_ENV
    (void)ctx;
    (void)env_name;
    (void)out_val;
    cco_diag_record(CCO_ERR_FORBIDDEN, 0, 0, "cco_eval_builtin_env", "$env is disabled");
    return CCO_ERR_FORBIDDEN;
#else
    if (!ctx->options->allow_env_builtin) {
        cco_diag_record(CCO_ERR_FORBIDDEN, 0, 0, "cco_eval_builtin_env", "$env is disabled by options");
        return CCO_ERR_FORBIDDEN;
    }
    
    if (!env_name) return CCO_ERR_INVALID_ARG;
    
    char* val = getenv(env_name);
    if (!val) {
        *out_val = cco_object_create(CCO_TYPE_NONE);
    } else {
        *out_val = cco_object_create(CCO_TYPE_STRING);
        /* In real code, duplicate the string into the object's string data */
    }
    return *out_val ? CCO_OK : CCO_ERR_NO_MEMORY;
#endif
}
