#include "eval.h"
#include "object.h"
#include <string.h>
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
    
    if (!expr_ast) return CCO_ERR_INVALID_ARG;
    *out_val = cco_object_clone(expr_ast);
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
    
    if (!format_str) return CCO_ERR_INVALID_ARG;
    *out_val = cco_object_create(CCO_TYPE_STRING);
    if (*out_val) {
        size_t len = strlen(format_str);
        char* sdup = malloc(len + 1);
        if (sdup) {
            memcpy(sdup, format_str, len + 1);
            (*out_val)->as.string.data = sdup;
            (*out_val)->as.string.length = len;
        } else {
            cco_object_release(*out_val);
            *out_val = NULL;
            return CCO_ERR_NO_MEMORY;
        }
    }
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
        if (*out_val) {
            size_t len = strlen(val);
            char* sdup = malloc(len + 1);
            if (sdup) {
                memcpy(sdup, val, len + 1);
                (*out_val)->as.string.data = sdup;
                (*out_val)->as.string.length = len;
            } else {
                cco_object_release(*out_val);
                *out_val = NULL;
                return CCO_ERR_NO_MEMORY;
            }
        }
    }
    return *out_val ? CCO_OK : CCO_ERR_NO_MEMORY;
#endif
}
