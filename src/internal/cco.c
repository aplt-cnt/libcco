#include <cnt/cco.h>
#include "parser.h"
#include "serializer.h"
#include "arena.h"
#include "diag.h"
#include "strbuf.h"

#include <stdlib.h>
#include <string.h>

cco_object_t* cco_parse_string(const char* src, size_t len, const cco_parse_options_t* opts)
{
    if (!src) return NULL;
    
    cco_diag_clear();
    
    cco_arena_t arena;
    cco_arena_init(&arena, opts ? opts->max_total_allocation : 0);
    
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
