#include <cnt/cco.h>
#include "parser.h"
#include "serializer.h"
#include "arena.h"
#include "diag.h"
#include "strbuf.h"

#include <stdlib.h>
#include <string.h>

void cco_parse_options_init(cco_parse_options_t* opts)
{
    if (!opts) return;
    memset(opts, 0, sizeof(cco_parse_options_t));
    opts->max_depth = 256;
    opts->max_string_length = 1048576;
    opts->max_document_size = 10485760;
    opts->max_total_allocation = 10485760;
    opts->max_instantiations = 1000;
    opts->max_steps = 10000;
    opts->allow_colon_instantiation = true;
    opts->strict_typing = true;
    opts->restrict_filesystem = true;
    opts->base_dir = ".";
#ifdef CCO_ENABLE_FORMAT
    opts->allow_format_builtin = true;
#endif
#ifdef CCO_ENABLE_EVAL
    opts->allow_eval = true;
#endif
#ifdef CCO_ENABLE_ENV
    opts->allow_env_builtin = true;
#endif
#ifdef CCO_ENABLE_STATIC_CALLS
    opts->allow_static_calls = true;
#endif
#ifdef CCO_ENABLE_CONSTRUCTORS
    opts->allow_constructors = true;
#endif
}

cco_object_t* cco_parse_string(const char* src, size_t len, const cco_parse_options_t* opts)
{
    if (!src) return NULL;
    
    cco_parse_options_t default_opts;
    if (!opts) {
        cco_parse_options_init(&default_opts);
        opts = &default_opts;
    }
    
    cco_diag_clear();
    
    cco_arena_t arena;
    cco_arena_init(&arena, opts->max_total_allocation);
    
    cco_parser_context_t ctx;
    cco_parser_init(&ctx, src, len, opts, &arena);
    
    cco_object_t* root = cco_parse_document(&ctx);
    
    cco_arena_destroy(&arena);
    return root;
}

char* cco_serialize_to_string(const cco_object_t* obj, bool pretty)
{
    if (!obj) return NULL;
    
    cco_arena_t arena;
    cco_arena_init(&arena, 0);
    
    cco_strbuf_t buf;
    cco_strbuf_init(&buf, &arena);
    
    cco_error_t err = cco_serialize(obj, &buf, pretty);
    
    char* result = NULL;
    if (err == CCO_OK && buf.data) {
        result = (char*)malloc(buf.length + 1);
        if (result) {
            memcpy(result, buf.data, buf.length);
            result[buf.length] = '\0';
        }
    }
    
    cco_arena_destroy(&arena);
    return result;
}

int cco_object_get_type(const cco_object_t* obj)
{
    if (!obj) return 0; /* CCO_TYPE_NONE */
    return (int)obj->type;
}

cco_error_t cco_get_last_error(void)
{
    return cco_diag_get_last_error();
}

void cco_clear_diagnostics(void)
{
    cco_diag_clear();
}
