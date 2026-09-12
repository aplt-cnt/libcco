#ifndef CNT_CCO_INTERNAL_CALL_H
#define CNT_CCO_INTERNAL_CALL_H

#include "object.h"
#include "parser.h"

/* Evaluates a static method call like #Math:square(5) */
cco_error_t cco_call_static_method(cco_parser_context_t* ctx,
                                   const char* target_name,
                                   const char* method_name, cco_array_t* args,
                                   cco_object_t** out_val);

/* Evaluates a colon instantiation call */
cco_error_t cco_call_colon_instantiation(cco_parser_context_t* ctx,
                                         const char* target_name,
                                         cco_object_t* arg,
                                         cco_object_t** out_val);

#endif /* CNT_CCO_INTERNAL_CALL_H */
