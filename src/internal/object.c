#include "object.h"

#include <stdlib.h>
#include <string.h>

cco_object_t* cco_object_create(cco_type_t type)
{
    cco_object_t* obj = (cco_object_t*)calloc(1, sizeof(cco_object_t));
    if (obj) {
        obj->type = type;
        obj->refcount = 1;
    }
    return obj;
}

void cco_object_retain(cco_object_t* obj)
{
    if (obj) {
        obj->refcount++;
    }
}

void cco_object_release(cco_object_t* obj)
{
    if (!obj) return;
    
    if (obj->refcount > 0) {
        obj->refcount--;
    }
    
    if (obj->refcount == 0) {
        switch (obj->type) {
            case CCO_TYPE_STRING:
                free(obj->as.string.data);
                break;
            case CCO_TYPE_ARRAY:
                for (size_t i = 0; i < obj->as.array.count; i++) {
                    cco_object_release(obj->as.array.items[i]);
                }
                free(obj->as.array.items);
                break;
            case CCO_TYPE_MAP:
                for (size_t i = 0; i < obj->as.map.count; i++) {
                    free(obj->as.map.keys[i]);
                    cco_object_release(obj->as.map.values[i]);
                }
                free(obj->as.map.keys);
                free(obj->as.map.values);
                break;
            case CCO_TYPE_TEMPLATE_INSTANCE:
                for (size_t i = 0; i < obj->as.tmpl_inst.fields.count; i++) {
                    free(obj->as.tmpl_inst.fields.keys[i]);
                    cco_object_release(obj->as.tmpl_inst.fields.values[i]);
                }
                free(obj->as.tmpl_inst.fields.keys);
                free(obj->as.tmpl_inst.fields.values);
                break;
            default:
                break;
        }
        free(obj);
    }
}

static char* cco_strdup(const char* s)
{
    if (!s) return NULL;
    size_t len = strlen(s);
    char* copy = (char*)malloc(len + 1);
    if (copy) {
        memcpy(copy, s, len + 1);
    }
    return copy;
}

cco_object_t* cco_object_clone(const cco_object_t* obj)
{
    if (!obj) return NULL;
    
    cco_object_t* copy = cco_object_create(obj->type);
    if (!copy) return NULL;
    
    switch (obj->type) {
        case CCO_TYPE_NONE:
            break;
        case CCO_TYPE_BOOLEAN:
            copy->as.boolean = obj->as.boolean;
            break;
        case CCO_TYPE_INTEGER:
            copy->as.integer = obj->as.integer;
            break;
        case CCO_TYPE_FLOAT:
            copy->as.floating = obj->as.floating;
            break;
        case CCO_TYPE_STRING:
            copy->as.string.length = obj->as.string.length;
            if (copy->as.string.length > 0) {
                copy->as.string.data = (char*)malloc(copy->as.string.length + 1);
                memcpy(copy->as.string.data, obj->as.string.data, copy->as.string.length + 1);
            }
            break;
        case CCO_TYPE_ARRAY:
            copy->as.array.count = obj->as.array.count;
            copy->as.array.capacity = obj->as.array.capacity;
            if (copy->as.array.capacity > 0) {
                copy->as.array.items = (cco_object_t**)malloc(sizeof(cco_object_t*) * copy->as.array.capacity);
                for (size_t i = 0; i < copy->as.array.count; i++) {
                    copy->as.array.items[i] = cco_object_clone(obj->as.array.items[i]);
                }
            }
            break;
        case CCO_TYPE_MAP:
            copy->as.map.count = obj->as.map.count;
            copy->as.map.capacity = obj->as.map.capacity;
            if (copy->as.map.capacity > 0) {
                copy->as.map.keys = (char**)malloc(sizeof(char*) * copy->as.map.capacity);
                copy->as.map.values = (cco_object_t**)malloc(sizeof(cco_object_t*) * copy->as.map.capacity);
                for (size_t i = 0; i < copy->as.map.count; i++) {
                    copy->as.map.keys[i] = cco_strdup(obj->as.map.keys[i]);
                    copy->as.map.values[i] = cco_object_clone(obj->as.map.values[i]);
                }
            }
            break;
        case CCO_TYPE_TEMPLATE_INSTANCE:
            copy->as.tmpl_inst.template_name = obj->as.tmpl_inst.template_name; /* interned pointer */
            copy->as.tmpl_inst.fields.count = obj->as.tmpl_inst.fields.count;
            copy->as.tmpl_inst.fields.capacity = obj->as.tmpl_inst.fields.capacity;
            if (copy->as.tmpl_inst.fields.capacity > 0) {
                copy->as.tmpl_inst.fields.keys = (char**)malloc(sizeof(char*) * copy->as.tmpl_inst.fields.capacity);
                copy->as.tmpl_inst.fields.values = (cco_object_t**)malloc(sizeof(cco_object_t*) * copy->as.tmpl_inst.fields.capacity);
                for (size_t i = 0; i < copy->as.tmpl_inst.fields.count; i++) {
                    copy->as.tmpl_inst.fields.keys[i] = cco_strdup(obj->as.tmpl_inst.fields.keys[i]);
                    copy->as.tmpl_inst.fields.values[i] = cco_object_clone(obj->as.tmpl_inst.fields.values[i]);
                }
            }
            break;
    }
    return copy;
}

bool cco_object_equals(const cco_object_t* a, const cco_object_t* b)
{
    if (a == b) return true;
    if (!a || !b) return false;
    if (a->type != b->type) return false;
    
    switch (a->type) {
        case CCO_TYPE_NONE:
            return true;
        case CCO_TYPE_BOOLEAN:
            return a->as.boolean == b->as.boolean;
        case CCO_TYPE_INTEGER:
            return a->as.integer == b->as.integer;
        case CCO_TYPE_FLOAT:
            return a->as.floating == b->as.floating;
        case CCO_TYPE_STRING:
            if (a->as.string.length != b->as.string.length) return false;
            return memcmp(a->as.string.data, b->as.string.data, a->as.string.length) == 0;
        case CCO_TYPE_ARRAY:
            if (a->as.array.count != b->as.array.count) return false;
            for (size_t i = 0; i < a->as.array.count; i++) {
                if (!cco_object_equals(a->as.array.items[i], b->as.array.items[i])) return false;
            }
            return true;
        case CCO_TYPE_MAP:
            /* Assuming keys are ordered or we just check sequential equality for simplicity here.
               Proper map comparison would check key presence and value match regardless of order.
               Since CCO maps preserve insertion order usually, we will do sequential check. */
            if (a->as.map.count != b->as.map.count) return false;
            for (size_t i = 0; i < a->as.map.count; i++) {
                if (strcmp(a->as.map.keys[i], b->as.map.keys[i]) != 0) return false;
                if (!cco_object_equals(a->as.map.values[i], b->as.map.values[i])) return false;
            }
            return true;
        case CCO_TYPE_TEMPLATE_INSTANCE:
            if (a->as.tmpl_inst.template_name != b->as.tmpl_inst.template_name) return false;
            if (a->as.tmpl_inst.fields.count != b->as.tmpl_inst.fields.count) return false;
            for (size_t i = 0; i < a->as.tmpl_inst.fields.count; i++) {
                if (strcmp(a->as.tmpl_inst.fields.keys[i], b->as.tmpl_inst.fields.keys[i]) != 0) return false;
                if (!cco_object_equals(a->as.tmpl_inst.fields.values[i], b->as.tmpl_inst.fields.values[i])) return false;
            }
            return true;
    }
    return false;
}
