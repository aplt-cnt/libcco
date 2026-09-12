#ifndef CNT_CCO_INTERNAL_INST_H
#define CNT_CCO_INTERNAL_INST_H

#include "object.h"
#include "parser.h"
#include "symtab.h"

/* State context for instantiation limits */
typedef struct
{
    size_t inst_count;
} cco_inst_context_t;

/* Instantiates a template by positional arguments */
cco_error_t cco_instantiate_positional(cco_parser_context_t* ctx,
                                       cco_symtab_t* symtab,
                                       cco_inst_context_t* inst_ctx,
                                       const char* tmpl_name, cco_array_t* args,
                                       cco_object_t** out_obj);

/* Instantiates a template by named arguments */
cco_error_t cco_instantiate_named(cco_parser_context_t* ctx,
                                  cco_symtab_t* symtab,
                                  cco_inst_context_t* inst_ctx,
                                  const char* tmpl_name, cco_map_t* args,
                                  cco_object_t** out_obj);

/* Custom constructor ($function.@) */
cco_error_t cco_instantiate_custom(cco_parser_context_t* ctx,
                                   cco_symtab_t* symtab,
                                   cco_inst_context_t* inst_ctx,
                                   const char* tmpl_name, cco_object_t* arg,
                                   cco_object_t** out_obj);

#endif /* CNT_CCO_INTERNAL_INST_H */
