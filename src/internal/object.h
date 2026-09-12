#ifndef CNT_CCO_INTERNAL_OBJECT_H
#define CNT_CCO_INTERNAL_OBJECT_H

#include <stddef.h>
#include <stdbool.h>
#include <stdint.h>
#include <cnt/cco_error.h>

typedef enum {
    CCO_TYPE_NONE = 0,
    CCO_TYPE_BOOLEAN,
    CCO_TYPE_INTEGER,
    CCO_TYPE_FLOAT,
    CCO_TYPE_STRING,
    CCO_TYPE_ARRAY,
    CCO_TYPE_MAP,
    CCO_TYPE_TEMPLATE_INSTANCE
} cco_type_t;

typedef struct cco_object_s cco_object_t;

/* Array structure */
typedef struct {
    cco_object_t** items;
    size_t count;
    size_t capacity;
} cco_array_t;

/* Map structure (wrapper around hash table conceptually) */
typedef struct {
    /* Using simple array of kv pairs for the layout, the parser will wrap hash */
    char** keys;
    cco_object_t** values;
    size_t count;
    size_t capacity;
} cco_map_t;

/* Template instance */
typedef struct {
    const char* template_name;
    cco_map_t fields;
} cco_template_instance_t;

struct cco_object_s {
    cco_type_t type;
    size_t refcount; /* Non-atomic reference counting */
    
    union {
        bool boolean;
        int64_t integer;
        double floating;
        struct {
            char* data;
            size_t length;
        } string;
        cco_array_t array;
        cco_map_t map;
        cco_template_instance_t tmpl_inst;
    } as;
};

/* Compile-time assertion on object size to prevent bloat */
#if defined(__STDC_VERSION__) && __STDC_VERSION__ >= 201112L
_Static_assert(sizeof(cco_object_t) <= 64, "cco_object_t exceeds maximum allowed size (64 bytes)");
#endif

/* Creates a new object with refcount 1 */
cco_object_t* cco_object_create(cco_type_t type);

/* Increments reference count */
void cco_object_retain(cco_object_t* obj);

/* Decrements reference count, frees if 0 */
void cco_object_release(cco_object_t* obj);

/* Performs a deep copy */
cco_object_t* cco_object_clone(const cco_object_t* obj);

/* Deep comparison. Returns true if identical */
bool cco_object_equals(const cco_object_t* a, const cco_object_t* b);

#endif /* CNT_CCO_INTERNAL_OBJECT_H */
