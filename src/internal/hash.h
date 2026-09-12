#ifndef CNT_CCO_INTERNAL_HASH_H
#define CNT_CCO_INTERNAL_HASH_H

#include <stddef.h>

#include <cnt/cco_error.h>

#include "arena.h"

typedef struct
{
    const char* key;
} cco_hash_entry_t;

typedef struct
{
    cco_hash_entry_t* entries;
    size_t capacity;
    size_t count;
    cco_arena_t* arena;
} cco_intern_table_t;

/* Initializes the intern table */
void cco_intern_table_init(cco_intern_table_t* table, cco_arena_t* arena);

/* Interns a string. If the string is already in the table, returns the existing
   pointer. Otherwise, allocates a copy in the arena and inserts it. Returns
   NULL on OOM. */
const char* cco_intern_string(cco_intern_table_t* table, const char* str,
                              size_t len);

/* Destroys the table structure (but strings remain in the arena) */
void cco_intern_table_destroy(cco_intern_table_t* table);

#endif /* CNT_CCO_INTERNAL_HASH_H */
