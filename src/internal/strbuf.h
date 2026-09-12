#ifndef CNT_CCO_INTERNAL_STRBUF_H
#define CNT_CCO_INTERNAL_STRBUF_H

#include <stddef.h>

#include <cnt/cco_error.h>

#include "arena.h"

typedef struct
{
    char* data;
    size_t length;
    size_t capacity;
    cco_arena_t* arena;
} cco_strbuf_t;

/* Initializes string buffer using arena for allocation */
void cco_strbuf_init(cco_strbuf_t* sb, cco_arena_t* arena);

/* Appends text of given length */
cco_error_t cco_strbuf_append(cco_strbuf_t* sb, const char* str, size_t len);

/* Appends formatted text */
cco_error_t cco_strbuf_appendf(cco_strbuf_t* sb, const char* fmt, ...);

#endif /* CNT_CCO_INTERNAL_STRBUF_H */
