#include "hash.h"

#include <stdint.h>
#include <string.h>
#include <stdlib.h>

#define FNV_OFFSET_BASIS 14695981039346656037ULL
#define FNV_PRIME 1099511628211ULL
#define HASH_INIT_CAPACITY 16

static uint64_t cco_hash_fnv1a(const char* str, size_t len)
{
    uint64_t hash = FNV_OFFSET_BASIS;
    for (size_t i = 0; i < len; i++) {
        hash ^= (uint8_t)str[i];
        hash *= FNV_PRIME;
    }
    return hash;
}

void cco_intern_table_init(cco_intern_table_t* table, cco_arena_t* arena)
{
    if (table) {
        table->entries = NULL;
        table->capacity = 0;
        table->count = 0;
        table->arena = arena;
    }
}

static cco_error_t cco_intern_table_resize(cco_intern_table_t* table)
{
    size_t new_cap = table->capacity == 0 ? HASH_INIT_CAPACITY : table->capacity * 2;
    if (new_cap < table->capacity) {
        return CCO_ERR_NO_MEMORY;
    }

    cco_hash_entry_t* new_entries = (cco_hash_entry_t*)calloc(new_cap, sizeof(cco_hash_entry_t));
    if (!new_entries) {
        return CCO_ERR_NO_MEMORY;
    }

    for (size_t i = 0; i < table->capacity; i++) {
        if (table->entries[i].key) {
            const char* key = table->entries[i].key;
            size_t len = strlen(key);
            uint64_t hash = cco_hash_fnv1a(key, len);
            size_t idx = hash % new_cap;
            
            while (new_entries[idx].key != NULL) {
                idx = (idx + 1) % new_cap;
            }
            new_entries[idx].key = key;
        }
    }

    free(table->entries);
    table->entries = new_entries;
    table->capacity = new_cap;
    return CCO_OK;
}

const char* cco_intern_string(cco_intern_table_t* table, const char* str, size_t len)
{
    if (!table || !str) return NULL;

    if (table->count >= (table->capacity * 3) / 4) {
        if (cco_intern_table_resize(table) != CCO_OK) {
            return NULL;
        }
    }

    uint64_t hash = cco_hash_fnv1a(str, len);
    size_t idx = hash % table->capacity;

    while (table->entries[idx].key != NULL) {
        const char* key = table->entries[idx].key;
        if (strncmp(key, str, len) == 0 && key[len] == '\0') {
            return key;
        }
        idx = (idx + 1) % table->capacity;
    }

    char* copy = (char*)cco_arena_alloc(table->arena, len + 1);
    if (!copy) {
        return NULL;
    }
    memcpy(copy, str, len);
    copy[len] = '\0';

    table->entries[idx].key = copy;
    table->count++;

    return copy;
}

void cco_intern_table_destroy(cco_intern_table_t* table)
{
    if (table) {
        free(table->entries);
        table->entries = NULL;
        table->capacity = 0;
        table->count = 0;
    }
}
