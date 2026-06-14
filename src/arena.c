#include <cnt/cco_p.h>
#include <stdlib.h>
#include <string.h>

#define CCO_ARENA_DEFAULT_CAP (64 * 1024)

void cco_arena_init(cco_arena_t *a, size_t cap) {
    if (cap == 0) cap = CCO_ARENA_DEFAULT_CAP;
    a->memory   = (char*)malloc(cap);
    a->capacity = a->memory ? cap : 0;
    a->offset   = 0;
}

void *cco_arena_alloc(cco_arena_t *a, size_t size) {
    if (!a->memory || a->offset + size > a->capacity) {
        return NULL;
    }
    void *ptr = a->memory + a->offset;
    a->offset += size;
    memset(ptr, 0, size);
    return ptr;
}

void cco_arena_reset(cco_arena_t *a) {
    a->offset = 0;
}

void cco_arena_free(cco_arena_t *a) {
    free(a->memory);
    a->memory   = NULL;
    a->capacity = 0;
    a->offset   = 0;
}
