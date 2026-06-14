#include <cnt/cco_p.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/*============================================================================
 * Internal helpers
 *============================================================================*/

/* Grow a field array to at least 'min' capacity */
int cco_grow_fields(cco_field_t **fields, size_t *cap, size_t min) {
    if (min <= *cap) return 0;
    size_t new_cap = *cap ? *cap * 2 : 8;
    while (new_cap < min) new_cap *= 2;
    cco_field_t *p = (cco_field_t*)realloc(*fields, new_cap * sizeof(cco_field_t));
    if (!p) return -1;
    memset(p + *cap, 0, (new_cap - *cap) * sizeof(cco_field_t));
    *fields = p;
    *cap    = new_cap;
    return 0;
}

/* Linear search for key in ordered field array. Returns index in *out. */
int cco_field_find(const cco_field_t *fields, size_t count,
                   const char *key, size_t *out) {
    for (size_t i = 0; i < count; i++) {
        if (strcmp(fields[i].key, key) == 0) {
            if (out) *out = i;
            return 1;
        }
    }
    return 0;
}

static char* safe_strdup(const char *s) {
    if (!s) return NULL;
    size_t n = strlen(s) + 1;
    char *p = (char*)malloc(n);
    if (p) memcpy(p, s, n);
    return p;
}

/*============================================================================
 * Object creation / destruction
 *============================================================================*/

cco_object_t* cco_object_new(void) {
    cco_object_t *obj = (cco_object_t*)calloc(1, sizeof(cco_object_t));
    if (obj) obj->type = CCO_TYPE_MAP;
    return obj;
}

cco_object_t* cco_none_new(void) {
    cco_object_t *obj = (cco_object_t*)calloc(1, sizeof(cco_object_t));
    if (obj) obj->type = CCO_TYPE_NONE;
    return obj;
}

cco_object_t* cco_string_new(const char *s) {
    if (!s) return NULL;
    cco_object_t *obj = (cco_object_t*)calloc(1, sizeof(cco_object_t));
    if (!obj) return NULL;
    obj->type        = CCO_TYPE_STRING;
    obj->data.string.data = safe_strdup(s);
    obj->data.string.len  = s ? strlen(s) : 0;
    if (s && !obj->data.string.data) { free(obj); return NULL; }
    return obj;
}

cco_object_t* cco_int_new(long long i) {
    cco_object_t *obj = (cco_object_t*)calloc(1, sizeof(cco_object_t));
    if (obj) { obj->type = CCO_TYPE_INT; obj->data.int_val = i; }
    return obj;
}

cco_object_t* cco_float_new(double f) {
    cco_object_t *obj = (cco_object_t*)calloc(1, sizeof(cco_object_t));
    if (obj) { obj->type = CCO_TYPE_FLOAT; obj->data.float_val = f; }
    return obj;
}

cco_object_t* cco_bool_new(int val) {
    cco_object_t *obj = (cco_object_t*)calloc(1, sizeof(cco_object_t));
    if (obj) { obj->type = CCO_TYPE_BOOL; obj->data.bool_val = val ? 1 : 0; }
    return obj;
}

cco_object_t* cco_template_instance_new(const char *template_name) {
    cco_object_t *obj = (cco_object_t*)calloc(1, sizeof(cco_object_t));
    if (!obj) return NULL;
    obj->type          = CCO_TYPE_TEMPLATE_INSTANCE;
    obj->template_name = safe_strdup(template_name);
    if (template_name && !obj->template_name) { free(obj); return NULL; }
    return obj;
}

static void cco_object_free_internal(cco_object_t *obj) {
    if (!obj) return;
    if (obj->type == CCO_TYPE_STRING) {
        free(obj->data.string.data);
    } else if (obj->type == CCO_TYPE_ARRAY && obj->data.array) {
        cco_array_free(obj->data.array);
    } else if (obj->type == CCO_TYPE_MAP || obj->type == CCO_TYPE_TEMPLATE_INSTANCE) {
        for (size_t i = 0; i < obj->field_count; i++) {
            free(obj->fields[i].key);
            cco_object_free_internal(obj->fields[i].value);
        }
        free(obj->fields);
    }
    free(obj->template_name);
    free(obj);
}

void cco_object_free(cco_object_t *obj) {
    cco_object_free_internal(obj);
}

/*============================================================================
 * Deep copy
 *============================================================================*/

static cco_object_t* cco_object_copy_internal(const cco_object_t *obj) {
    if (!obj) return NULL;
    switch (obj->type) {
    case CCO_TYPE_NONE:
        return cco_none_new();
    case CCO_TYPE_BOOL:
        return cco_bool_new(obj->data.bool_val);
    case CCO_TYPE_INT:
        return cco_int_new(obj->data.int_val);
    case CCO_TYPE_FLOAT:
        return cco_float_new(obj->data.float_val);
    case CCO_TYPE_STRING:
        return cco_string_new(obj->data.string.data);
    case CCO_TYPE_ARRAY: {
        if (!obj->data.array) return cco_array_wrap(cco_array_new());
        cco_array_t *arr = cco_array_copy(obj->data.array);
        return arr ? cco_array_wrap(arr) : NULL;
    }
    case CCO_TYPE_MAP:
    case CCO_TYPE_TEMPLATE_INSTANCE: {
        cco_object_t *copy;
        if (obj->type == CCO_TYPE_TEMPLATE_INSTANCE) {
            copy = cco_template_instance_new(obj->template_name);
        } else {
            copy = cco_object_new();
        }
        if (!copy) return NULL;
        for (size_t i = 0; i < obj->field_count; i++) {
            cco_object_t *vcopy = cco_object_copy_internal(obj->fields[i].value);
            if (!vcopy) { cco_object_free_internal(copy); return NULL; }
            cco_object_set(copy, obj->fields[i].key, vcopy);
        }
        return copy;
    }
    }
    return NULL;
}

cco_object_t* cco_object_copy(const cco_object_t *obj) {
    return cco_object_copy_internal(obj);
}

/*============================================================================
 * Array
 *============================================================================*/

cco_array_t* cco_array_new(void) {
    cco_array_t *arr = (cco_array_t*)calloc(1, sizeof(cco_array_t));
    return arr;
}

void cco_array_free(cco_array_t *arr) {
    if (!arr) return;
    for (size_t i = 0; i < arr->count; i++)
        cco_object_free_internal(arr->items[i]);
    free(arr->items);
    free(arr);
}

cco_array_t* cco_array_copy(const cco_array_t *arr) {
    if (!arr) return NULL;
    cco_array_t *copy = cco_array_new();
    if (!copy) return NULL;
    for (size_t i = 0; i < arr->count; i++) {
        cco_object_t *vcopy = cco_object_copy_internal(arr->items[i]);
        if (!vcopy) { cco_array_free(copy); return NULL; }
        if (cco_array_add(copy, vcopy) != 0) {
            cco_object_free_internal(vcopy);
            cco_array_free(copy);
            return NULL;
        }
    }
    return copy;
}

static int cco_array_grow(cco_array_t *arr, size_t min) {
    if (min <= arr->capacity) return 0;
    size_t new_cap = arr->capacity ? arr->capacity * 2 : 8;
    while (new_cap < min) new_cap *= 2;
    cco_object_t **p = (cco_object_t**)realloc(arr->items, new_cap * sizeof(cco_object_t*));
    if (!p) return -1;
    arr->items    = p;
    arr->capacity = new_cap;
    return 0;
}

int cco_array_add(cco_array_t *arr, cco_object_t *value) {
    if (!arr) return CCO_ERR("cco_array_add: NULL array");
    if (cco_array_grow(arr, arr->count + 1) != 0)
        return CCO_ERR("cco_array_add: out of memory");
    arr->items[arr->count++] = value;
    return 0;
}

int cco_array_insert(cco_array_t *arr, size_t index, cco_object_t *value) {
    if (!arr) return CCO_ERR("cco_array_insert: NULL array");
    if (index > arr->count)
        return CCO_ERR("cco_array_insert: index %zu out of bounds (size %zu)", index, arr->count);
    if (cco_array_grow(arr, arr->count + 1) != 0)
        return CCO_ERR("cco_array_insert: out of memory");
    memmove(&arr->items[index + 1], &arr->items[index],
            (arr->count - index) * sizeof(cco_object_t*));
    arr->items[index] = value;
    arr->count++;
    return 0;
}

int cco_array_remove(cco_array_t *arr, size_t index) {
    if (!arr) return CCO_ERR("cco_array_remove: NULL array");
    if (index >= arr->count)
        return CCO_ERR("cco_array_remove: index %zu out of bounds (size %zu)", index, arr->count);
    cco_object_free_internal(arr->items[index]);
    memmove(&arr->items[index], &arr->items[index + 1],
            (arr->count - index - 1) * sizeof(cco_object_t*));
    arr->count--;
    return 0;
}

cco_object_t* cco_array_get(const cco_array_t *arr, size_t index) {
    if (!arr || index >= arr->count) return NULL;
    return arr->items[index];
}

size_t cco_array_size(const cco_array_t *arr) {
    return arr ? arr->count : 0;
}

void cco_array_clear(cco_array_t *arr) {
    if (!arr) return;
    for (size_t i = 0; i < arr->count; i++)
        cco_object_free_internal(arr->items[i]);
    arr->count = 0;
}

cco_object_t* cco_array_wrap(cco_array_t *arr) {
    if (!arr) return NULL;
    cco_object_t *obj = (cco_object_t*)calloc(1, sizeof(cco_object_t));
    if (!obj) return NULL;
    obj->type       = CCO_TYPE_ARRAY;
    obj->data.array = arr;
    return obj;
}

/*============================================================================
 * Map operations
 *============================================================================*/

int cco_object_set(cco_object_t *obj, const char *key, cco_object_t *value) {
    if (!obj || !key) return CCO_ERR("cco_object_set: NULL argument");
    if (obj->type != CCO_TYPE_MAP && obj->type != CCO_TYPE_TEMPLATE_INSTANCE)
        return CCO_ERR("cco_object_set: object is not a map");

    /* Check if key exists */
    size_t idx;
    if (cco_field_find(obj->fields, obj->field_count, key, &idx)) {
        /* Replace existing */
        if (obj->fields[idx].value)
            cco_object_free_internal(obj->fields[idx].value);
        obj->fields[idx].value = value;
        return 0;
    }

    /* Append new */
    if (cco_grow_fields(&obj->fields, &obj->field_capacity, obj->field_count + 1) != 0)
        return CCO_ERR("cco_object_set: out of memory");
    obj->fields[obj->field_count].key   = safe_strdup(key);
    obj->fields[obj->field_count].value = value;
    obj->field_count++;
    return 0;
}

int cco_object_del(cco_object_t *obj, const char *key) {
    if (!obj || !key) return -1;
    if (obj->type != CCO_TYPE_MAP && obj->type != CCO_TYPE_TEMPLATE_INSTANCE) return -1;
    size_t idx;
    if (!cco_field_find(obj->fields, obj->field_count, key, &idx)) return -1;
    free(obj->fields[idx].key);
    cco_object_free_internal(obj->fields[idx].value);
    memmove(&obj->fields[idx], &obj->fields[idx + 1],
            (obj->field_count - idx - 1) * sizeof(cco_field_t));
    obj->field_count--;
    return 0;
}

void cco_object_clear(cco_object_t *obj) {
    if (!obj) return;
    if (obj->type != CCO_TYPE_MAP && obj->type != CCO_TYPE_TEMPLATE_INSTANCE) return;
    for (size_t i = 0; i < obj->field_count; i++) {
        free(obj->fields[i].key);
        cco_object_free_internal(obj->fields[i].value);
    }
    obj->field_count = 0;
}

cco_object_t* cco_object_get(const cco_object_t *obj, const char *key) {
    if (!obj || !key) return NULL;
    if (obj->type != CCO_TYPE_MAP && obj->type != CCO_TYPE_TEMPLATE_INSTANCE)
        return NULL;
    size_t idx;
    if (!cco_field_find(obj->fields, obj->field_count, key, &idx)) return NULL;
    return obj->fields[idx].value;
}

int cco_object_has_key(const cco_object_t *obj, const char *key) {
    if (!obj || !key) return 0;
    if (obj->type != CCO_TYPE_MAP && obj->type != CCO_TYPE_TEMPLATE_INSTANCE) return 0;
    return cco_field_find(obj->fields, obj->field_count, key, NULL);
}

size_t cco_object_size(const cco_object_t *obj) {
    if (!obj) return 0;
    if (obj->type != CCO_TYPE_MAP && obj->type != CCO_TYPE_TEMPLATE_INSTANCE) return 0;
    return obj->field_count;
}

const char* cco_object_template_name(const cco_object_t *obj) {
    if (!obj || obj->type != CCO_TYPE_TEMPLATE_INSTANCE) return NULL;
    return obj->template_name;
}

/*============================================================================
 * Iterator
 *============================================================================*/

cco_map_iter_t* cco_map_iter_new(const cco_object_t *map) {
    if (!map || (map->type != CCO_TYPE_MAP && map->type != CCO_TYPE_TEMPLATE_INSTANCE)) {
        CCO_ERR("cco_map_iter_new: not a map");
        return NULL;
    }
    cco_map_iter_t *iter = (cco_map_iter_t*)malloc(sizeof(cco_map_iter_t));
    if (!iter) return NULL;
    iter->map   = map;
    iter->index = 0;
    return iter;
}

void cco_map_iter_free(cco_map_iter_t *iter) {
    free(iter);
}

int cco_map_iter_next(cco_map_iter_t *iter, const char **key, cco_object_t **value) {
    if (!iter || iter->index >= iter->map->field_count) return 0;
    if (key)   *key   = iter->map->fields[iter->index].key;
    if (value) *value = iter->map->fields[iter->index].value;
    iter->index++;
    return 1;
}

/*============================================================================
 * Type queries and accessors
 *============================================================================*/

cco_value_type_t cco_object_type(const cco_object_t *obj) {
    return obj ? obj->type : CCO_TYPE_NONE;
}

const char* cco_string_get(const cco_object_t *obj) {
    if (!obj || obj->type != CCO_TYPE_STRING) return "";
    return obj->data.string.data ? obj->data.string.data : "";
}

long long cco_int_get(const cco_object_t *obj) {
    if (!obj || obj->type != CCO_TYPE_INT) return 0;
    return obj->data.int_val;
}

double cco_float_get(const cco_object_t *obj) {
    if (!obj || obj->type != CCO_TYPE_FLOAT) return 0.0;
    return obj->data.float_val;
}

int cco_bool_get(const cco_object_t *obj) {
    if (!obj || obj->type != CCO_TYPE_BOOL) return 0;
    return obj->data.bool_val;
}

int cco_is_none(const cco_object_t *obj) {
    return !obj || obj->type == CCO_TYPE_NONE;
}

const cco_array_t* cco_object_as_array(const cco_object_t *obj) {
    if (!obj || obj->type != CCO_TYPE_ARRAY) return NULL;
    return obj->data.array;
}
