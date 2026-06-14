#include <cnt/cco_p.h>
#include <stdlib.h>
#include <string.h>

/* Internal copy helper (defined after public constructors) */
static cco_type_expr_t* internal_copy(const cco_type_expr_t *src);

/*============================================================================
 * Constructor / Destructor
 *============================================================================*/

cco_type_expr_t* cco_type_primitive(cco_primitive_type_t prim) {
    cco_type_expr_t *te = (cco_type_expr_t*)calloc(1, sizeof(cco_type_expr_t));
    if (!te) return NULL;
    te->kind = CCO_TYPEKIND_PRIMITIVE;
    te->data.primitive = prim;
    return te;
}

cco_type_expr_t* cco_type_array(cco_type_expr_t *element_type) {
    if (!element_type) return NULL;
    cco_type_expr_t *te = (cco_type_expr_t*)calloc(1, sizeof(cco_type_expr_t));
    if (!te) { cco_type_expr_free(element_type); return NULL; }
    te->kind                    = CCO_TYPEKIND_ARRAY;
    te->data.array.element_type = element_type;
    return te;
}

cco_type_expr_t* cco_type_object(const cco_field_type_pair_t *fields, size_t count) {
    cco_type_expr_t *te = (cco_type_expr_t*)calloc(1, sizeof(cco_type_expr_t));
    if (!te) return NULL;
    te->kind = CCO_TYPEKIND_OBJECT;

    if (count > 0) {
        te->data.object.fields = (cco_field_type_pair_t*)calloc(count, sizeof(cco_field_type_pair_t));
        if (!te->data.object.fields) { free(te); return NULL; }
        for (size_t i = 0; i < count; i++) {
            te->data.object.fields[i].name = fields[i].name ? (char*)strdup(fields[i].name) : NULL;
            te->data.object.fields[i].type = fields[i].type ? internal_copy(fields[i].type) : NULL;
        }
        te->data.object.count = count;
    }
    return te;
}

cco_type_expr_t* cco_type_named(const char *name) {
    if (!name) return NULL;
    cco_type_expr_t *te = (cco_type_expr_t*)calloc(1, sizeof(cco_type_expr_t));
    if (!te) return NULL;
    te->kind = CCO_TYPEKIND_NAMED;
    te->data.named.name = strdup(name);
    if (!te->data.named.name) { free(te); return NULL; }
    return te;
}

cco_type_expr_t* cco_type_dyn(const char *base_template_name) {
    if (!base_template_name) return NULL;
    cco_type_expr_t *te = (cco_type_expr_t*)calloc(1, sizeof(cco_type_expr_t));
    if (!te) return NULL;
    te->kind = CCO_TYPEKIND_DYN;
    te->data.dyn.name = strdup(base_template_name);
    if (!te->data.dyn.name) { free(te); return NULL; }
    return te;
}

void cco_type_expr_free(cco_type_expr_t *te) {
    if (!te) return;
    switch (te->kind) {
    case CCO_TYPEKIND_ARRAY:
        cco_type_expr_free(te->data.array.element_type);
        break;
    case CCO_TYPEKIND_OBJECT:
        for (size_t i = 0; i < te->data.object.count; i++) {
            free((void*)te->data.object.fields[i].name);
            cco_type_expr_free(te->data.object.fields[i].type);
        }
        free(te->data.object.fields);
        break;
    case CCO_TYPEKIND_NAMED:
        free(te->data.named.name);
        break;
    case CCO_TYPEKIND_DYN:
        free(te->data.dyn.name);
        break;
    case CCO_TYPEKIND_PRIMITIVE:
        break;
    }
    free(te);
}

/*============================================================================
 * Accessors
 *============================================================================*/
cco_type_kind_t cco_type_expr_kind(const cco_type_expr_t *te) {
    return te ? te->kind : CCO_TYPEKIND_PRIMITIVE;
}

cco_primitive_type_t cco_type_expr_primitive(const cco_type_expr_t *te) {
    if (!te || te->kind != CCO_TYPEKIND_PRIMITIVE) return CCO_PRIM_NONE;
    return te->data.primitive;
}

const char* cco_type_expr_name(const cco_type_expr_t *te) {
    if (!te) return NULL;
    if (te->kind == CCO_TYPEKIND_NAMED) return te->data.named.name;
    if (te->kind == CCO_TYPEKIND_DYN)   return te->data.dyn.name;
    return NULL;
}

/*============================================================================
 * Copy / Compare
 *============================================================================*/

static cco_type_expr_t* internal_copy(const cco_type_expr_t *src) {
    if (!src) return NULL;
    switch (src->kind) {
    case CCO_TYPEKIND_PRIMITIVE:
        return cco_type_primitive(src->data.primitive);
    case CCO_TYPEKIND_ARRAY:
        return cco_type_array(internal_copy(src->data.array.element_type));
    case CCO_TYPEKIND_OBJECT:
        return cco_type_object(src->data.object.fields, src->data.object.count);
    case CCO_TYPEKIND_NAMED:
        return cco_type_named(src->data.named.name);
    case CCO_TYPEKIND_DYN:
        return cco_type_dyn(src->data.dyn.name);
    }
    return NULL;
}

cco_type_expr_t* cco_type_expr_copy(const cco_type_expr_t *te) {
    return internal_copy(te);
}

int cco_type_expr_equal(const cco_type_expr_t *a, const cco_type_expr_t *b) {
    if (!a || !b) return a == b;
    if (a->kind != b->kind) return 0;

    switch (a->kind) {
    case CCO_TYPEKIND_PRIMITIVE:
        return a->data.primitive == b->data.primitive;
    case CCO_TYPEKIND_ARRAY:
        return cco_type_expr_equal(a->data.array.element_type, b->data.array.element_type);
    case CCO_TYPEKIND_OBJECT:
        if (a->data.object.count != b->data.object.count) return 0;
        for (size_t i = 0; i < a->data.object.count; i++) {
            if (strcmp(a->data.object.fields[i].name, b->data.object.fields[i].name) != 0)
                return 0;
            if (!cco_type_expr_equal(a->data.object.fields[i].type, b->data.object.fields[i].type))
                return 0;
        }
        return 1;
    case CCO_TYPEKIND_NAMED:
        return strcmp(a->data.named.name, b->data.named.name) == 0;
    case CCO_TYPEKIND_DYN:
        return strcmp(a->data.dyn.name, b->data.dyn.name) == 0;
    }
    return 0;
}
