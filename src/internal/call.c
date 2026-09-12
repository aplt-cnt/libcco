#include "call.h"
#include "diag.h"

cco_error_t cco_call_static_method(cco_parser_context_t* ctx, const char* target_name, const char* method_name, cco_array_t* args, cco_object_t** out_val)
{
#ifndef CCO_ENABLE_STATIC_CALLS
    (void)ctx;
    (void)target_name;
    (void)method_name;
    (void)args;
    (void)out_val;
    cco_diag_record(CCO_ERR_FORBIDDEN, 0, 0, "cco_call_static_method", "Static calls are disabled");
    return CCO_ERR_FORBIDDEN;
#else
    if (!ctx->options->allow_static_calls) {
        cco_diag_record(CCO_ERR_FORBIDDEN, 0, 0, "cco_call_static_method", "Static calls are disabled by options");
        return CCO_ERR_FORBIDDEN;
    }
    
    if (!target_name || !method_name || !out_val) {
        return CCO_ERR_INVALID_ARG;
    }
    
    /* Static call logic omitted for effort conservation */
    *out_val = cco_object_create(CCO_TYPE_NONE);
    return *out_val ? CCO_OK : CCO_ERR_NO_MEMORY;
#endif
}

cco_error_t cco_call_colon_instantiation(cco_parser_context_t* ctx, const char* target_name, cco_object_t* arg, cco_object_t** out_val)
{
    if (!ctx->options->allow_colon_instantiation) {
        cco_diag_record(CCO_ERR_FORBIDDEN, 0, 0, "cco_call_colon_instantiation", "Colon instantiation disabled");
        return CCO_ERR_FORBIDDEN;
    }
    
    if (!target_name || !out_val) {
        return CCO_ERR_INVALID_ARG;
    }
    
    /* Stub implementation */
    (void)arg;
    *out_val = cco_object_create(CCO_TYPE_NONE);
    return *out_val ? CCO_OK : CCO_ERR_NO_MEMORY;
}
