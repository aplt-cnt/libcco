#include "inst.h"

#include "diag.h"

static cco_error_t check_inst_limit(cco_parser_context_t* ctx,
                                    cco_inst_context_t* inst_ctx)
{
    if (ctx->options->max_instantiations > 0)
    {
        if (inst_ctx->inst_count >= ctx->options->max_instantiations)
        {
            cco_diag_record(CCO_ERR_OUT_OF_RANGE, 0, 0, "check_inst_limit",
                            "Max instantiations exceeded");
            return CCO_ERR_OUT_OF_RANGE;
        }
    }
    inst_ctx->inst_count++;
    return CCO_OK;
}

cco_error_t cco_instantiate_positional(cco_parser_context_t* ctx,
                                       cco_symtab_t* symtab,
                                       cco_inst_context_t* inst_ctx,
                                       const char* tmpl_name, cco_array_t* args,
                                       cco_object_t** out_obj)
{
    (void)symtab;
    (void)args;

    cco_error_t err = check_inst_limit(ctx, inst_ctx);
    if (err != CCO_OK)
        return err;

    *out_obj = cco_object_create(CCO_TYPE_TEMPLATE_INSTANCE);
    if (!*out_obj)
        return CCO_ERR_NO_MEMORY;

    (*out_obj)->as.tmpl_inst.template_name = tmpl_name;
    return CCO_OK;
}

cco_error_t cco_instantiate_named(cco_parser_context_t* ctx,
                                  cco_symtab_t* symtab,
                                  cco_inst_context_t* inst_ctx,
                                  const char* tmpl_name, cco_map_t* args,
                                  cco_object_t** out_obj)
{
    (void)symtab;
    (void)args;

    cco_error_t err = check_inst_limit(ctx, inst_ctx);
    if (err != CCO_OK)
        return err;

    *out_obj = cco_object_create(CCO_TYPE_TEMPLATE_INSTANCE);
    if (!*out_obj)
        return CCO_ERR_NO_MEMORY;

    (*out_obj)->as.tmpl_inst.template_name = tmpl_name;
    return CCO_OK;
}

cco_error_t cco_instantiate_custom(cco_parser_context_t* ctx,
                                   cco_symtab_t* symtab,
                                   cco_inst_context_t* inst_ctx,
                                   const char* tmpl_name, cco_object_t* arg,
                                   cco_object_t** out_obj)
{
#ifndef CCO_ENABLE_CONSTRUCTORS
    (void)ctx;
    (void)symtab;
    (void)inst_ctx;
    (void)tmpl_name;
    (void)arg;
    (void)out_obj;
    cco_diag_record(CCO_ERR_FORBIDDEN, 0, 0, "cco_instantiate_custom",
                    "Custom constructors are disabled");
    return CCO_ERR_FORBIDDEN;
#else
    (void)symtab;
    (void)arg;

    if (!ctx->options->allow_constructors)
    {
        cco_diag_record(CCO_ERR_FORBIDDEN, 0, 0, "cco_instantiate_custom",
                        "Custom constructors are disabled by options");
        return CCO_ERR_FORBIDDEN;
    }

    cco_error_t err = check_inst_limit(ctx, inst_ctx);
    if (err != CCO_OK)
        return err;

    *out_obj = cco_object_create(CCO_TYPE_TEMPLATE_INSTANCE);
    if (!*out_obj)
        return CCO_ERR_NO_MEMORY;

    (*out_obj)->as.tmpl_inst.template_name = tmpl_name;
    return CCO_OK;
#endif
}
