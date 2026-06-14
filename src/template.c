#include <cnt/cco_p.h>
#include <stdlib.h>
#include <string.h>

/*============================================================================
 * Internal field entry
 *============================================================================*/
typedef struct cco_template_field {
    char        *name;
    cco_type_expr_t *type;
    cco_expr_t      *default_expr;
} cco_template_field_t;

/*============================================================================
 * Template
 *============================================================================*/
struct cco_template {
    char       *name;
    char       *parent_name;
    cco_template_field_t  *fields;
    size_t                  field_count;
    size_t                  field_capacity;
    cco_constructor_t     **constructors;
    size_t                  constructor_count;
    size_t                  constructor_capacity;
    cco_static_method_t   **static_methods;
    size_t                  static_method_count;
    size_t                  static_method_capacity;
};

cco_template_t* cco_template_new(const char *name, const char *parent_name) {
    if (!name) return NULL;
    cco_template_t *tmpl = (cco_template_t*)calloc(1, sizeof(cco_template_t));
    if (!tmpl) return NULL;
    tmpl->name = strdup(name);
    tmpl->parent_name = parent_name ? strdup(parent_name) : NULL;
    if (!tmpl->name) { free(tmpl); return NULL; }
    return tmpl;
}

void cco_template_free(cco_template_t *tmpl) {
    if (!tmpl) return;
    free(tmpl->name);
    free(tmpl->parent_name);
    for (size_t i = 0; i < tmpl->field_count; i++) {
        free(tmpl->fields[i].name);
        cco_type_expr_free(tmpl->fields[i].type);
        cco_expr_free(tmpl->fields[i].default_expr);
    }
    free(tmpl->fields);
    for (size_t i = 0; i < tmpl->constructor_count; i++)
        cco_constructor_free(tmpl->constructors[i]);
    free(tmpl->constructors);
    for (size_t i = 0; i < tmpl->static_method_count; i++)
        cco_static_method_free(tmpl->static_methods[i]);
    free(tmpl->static_methods);
    free(tmpl);
}

int cco_template_add_field(cco_template_t *tmpl, const char *name,
                           cco_type_expr_t *type, cco_expr_t *default_expr) {
    if (!tmpl || !name) { cco_type_expr_free(type); cco_expr_free(default_expr); return -1; }
    if (tmpl->field_count >= tmpl->field_capacity) {
        size_t new_cap = tmpl->field_capacity ? tmpl->field_capacity * 2 : 8;
        cco_template_field_t *p = (cco_template_field_t*)realloc(
            tmpl->fields, new_cap * sizeof(cco_template_field_t));
        if (!p) { cco_type_expr_free(type); cco_expr_free(default_expr); return -1; }
        tmpl->fields    = p;
        tmpl->field_capacity = new_cap;
    }
    size_t i = tmpl->field_count++;
    tmpl->fields[i].name         = strdup(name);
    tmpl->fields[i].type         = type;
    tmpl->fields[i].default_expr = default_expr;
    if (!tmpl->fields[i].name) { tmpl->field_count--; return -1; }
    return 0;
}

int cco_template_add_constructor(cco_template_t *tmpl, cco_constructor_t *ctor) {
    if (!tmpl || !ctor) return -1;
    if (tmpl->constructor_count >= tmpl->constructor_capacity) {
        size_t new_cap = tmpl->constructor_capacity ? tmpl->constructor_capacity * 2 : 4;
        cco_constructor_t **p = (cco_constructor_t**)realloc(
            tmpl->constructors, new_cap * sizeof(cco_constructor_t*));
        if (!p) return -1;
        tmpl->constructors    = p;
        tmpl->constructor_capacity = new_cap;
    }
    tmpl->constructors[tmpl->constructor_count++] = ctor;
    return 0;
}

int cco_template_add_static_method(cco_template_t *tmpl, cco_static_method_t *method) {
    if (!tmpl || !method) return -1;
    if (tmpl->static_method_count >= tmpl->static_method_capacity) {
        size_t new_cap = tmpl->static_method_capacity ? tmpl->static_method_capacity * 2 : 4;
        cco_static_method_t **p = (cco_static_method_t**)realloc(
            tmpl->static_methods, new_cap * sizeof(cco_static_method_t*));
        if (!p) return -1;
        tmpl->static_methods    = p;
        tmpl->static_method_capacity = new_cap;
    }
    tmpl->static_methods[tmpl->static_method_count++] = method;
    return 0;
}

cco_template_t* cco_template_get_parent(const cco_template_t *tmpl, const cco_symbol_table_t *st) {
    if (!tmpl || !tmpl->parent_name || !st) return NULL;
    return cco_symbol_table_lookup_template(st, tmpl->parent_name);
}

const char* cco_template_get_name(const cco_template_t *tmpl) {
    return tmpl ? tmpl->name : NULL;
}

const char* cco_template_get_parent_name(const cco_template_t *tmpl) {
    return tmpl ? tmpl->parent_name : NULL;
}

int cco_template_is_subtype(cco_symbol_table_t *st, const char *child, const char *base) {
    if (!st || !child || !base) return 0;
    if (strcmp(child, base) == 0) return 1; /* same type */

    cco_template_t *tmpl = cco_symbol_table_lookup_template(st, child);
    if (!tmpl) return 0;

    const char *parent = cco_template_get_parent_name(tmpl);
    while (parent) {
        if (strcmp(parent, base) == 0) return 1;
        tmpl = cco_symbol_table_lookup_template(st, parent);
        if (!tmpl) return 0;
        parent = cco_template_get_parent_name(tmpl);
    }
    return 0;
}

size_t cco_template_field_count(const cco_template_t *tmpl) {
    return tmpl ? tmpl->field_count : 0;
}

const char* cco_template_field_name(const cco_template_t *tmpl, size_t idx) {
    if (!tmpl || idx >= tmpl->field_count) return NULL;
    return tmpl->fields[idx].name;
}

cco_expr_t* cco_template_field_default(const cco_template_t *tmpl, size_t idx) {
    if (!tmpl || idx >= tmpl->field_count) return NULL;
    return tmpl->fields[idx].default_expr;
}

/*============================================================================
 * Constructor
 *============================================================================*/
struct cco_constructor {
    int               is_default;
    cco_param_list_t *params;
    cco_method_body_t *body;
};

cco_constructor_t* cco_constructor_default(void) {
    cco_constructor_t *ctor = (cco_constructor_t*)calloc(1, sizeof(cco_constructor_t));
    if (ctor) ctor->is_default = 1;
    return ctor;
}

cco_constructor_t* cco_constructor_new(cco_param_list_t *params, cco_method_body_t *body) {
    cco_constructor_t *ctor = (cco_constructor_t*)calloc(1, sizeof(cco_constructor_t));
    if (!ctor) { cco_param_list_free(params); cco_method_body_free(body); return NULL; }
    ctor->params = params;
    ctor->body   = body;
    return ctor;
}

void cco_constructor_free(cco_constructor_t *ctor) {
    if (!ctor) return;
    cco_param_list_free(ctor->params);
    cco_method_body_free(ctor->body);
    free(ctor);
}

/*============================================================================
 * Static method
 *============================================================================*/
struct cco_static_method {
    char                *name;
    int                  is_private;
    size_t               generic_count;
    cco_generic_param_t *generic_params;
    cco_param_list_t    *params;
    cco_type_expr_t     *return_type;
    cco_method_body_t   *body;
};

cco_static_method_t* cco_static_method_new(
    const char *name, int is_private,
    const cco_generic_param_t *generic_params, size_t generic_count,
    cco_param_list_t *params, cco_type_expr_t *return_type,
    cco_method_body_t *body)
{
    if (!name) { cco_param_list_free(params); cco_type_expr_free(return_type);
                 cco_method_body_free(body); return NULL; }
    cco_static_method_t *m = (cco_static_method_t*)calloc(1, sizeof(cco_static_method_t));
    if (!m) { cco_param_list_free(params); cco_type_expr_free(return_type);
              cco_method_body_free(body); return NULL; }
    m->name        = strdup(name);
    m->is_private  = is_private ? 1 : 0;
    m->params      = params;
    m->return_type = return_type;
    m->body        = body;

    if (generic_count > 0) {
        m->generic_params = (cco_generic_param_t*)calloc(generic_count, sizeof(cco_generic_param_t));
        if (m->generic_params) {
            for (size_t i = 0; i < generic_count; i++) {
                m->generic_params[i].name = generic_params[i].name ? strdup(generic_params[i].name) : NULL;
                m->generic_params[i].constraint = generic_params[i].constraint ?
                    cco_type_expr_copy(generic_params[i].constraint) : NULL;
            }
            m->generic_count = generic_count;
        }
    }
    return m;
}

void cco_static_method_free(cco_static_method_t *method) {
    if (!method) return;
    free(method->name);
    for (size_t i = 0; i < method->generic_count; i++) {
        free((void*)method->generic_params[i].name);
        cco_type_expr_free(method->generic_params[i].constraint);
    }
    free(method->generic_params);
    cco_param_list_free(method->params);
    cco_type_expr_free(method->return_type);
    cco_method_body_free(method->body);
    free(method);
}

/*============================================================================
 * Parameter list
 *============================================================================*/

struct cco_param_list {
    size_t           count;
    char           **names;
    cco_type_expr_t **types;
};

/*============================================================================
 * Constructor query (used by evaluator)
 *============================================================================*/
size_t cco_template_constructor_count(const cco_template_t *tmpl) {
    return tmpl ? tmpl->constructor_count : 0;
}

cco_constructor_t* cco_template_get_constructor(const cco_template_t *tmpl, size_t idx) {
    if (!tmpl || idx >= tmpl->constructor_count) return NULL;
    return tmpl->constructors[idx];
}

int cco_constructor_is_default(const cco_constructor_t *ctor) {
    return ctor ? ctor->is_default : 0;
}

size_t cco_constructor_param_count(const cco_constructor_t *ctor) {
    if (!ctor || !ctor->params) return 0;
    return ctor->params->count;
}

const char* cco_constructor_param_name(const cco_constructor_t *ctor, size_t idx) {
    if (!ctor || !ctor->params || idx >= ctor->params->count) return NULL;
    return ctor->params->names[idx];
}

cco_method_body_t* cco_constructor_get_body(const cco_constructor_t *ctor) {
    return ctor ? ctor->body : NULL;
}

/*============================================================================
 * Parameter list
 *============================================================================*/

cco_param_list_t* cco_param_list_new(size_t count, const char **names, cco_type_expr_t **types) {
    cco_param_list_t *pl = (cco_param_list_t*)calloc(1, sizeof(cco_param_list_t));
    if (!pl) return NULL;
    pl->count = count;
    if (count > 0) {
        pl->names = (char**)calloc(count, sizeof(char*));
        pl->types = (cco_type_expr_t**)calloc(count, sizeof(cco_type_expr_t*));
        if (!pl->names || !pl->types) { cco_param_list_free(pl); return NULL; }
        for (size_t i = 0; i < count; i++) {
            if (names && names[i]) pl->names[i] = strdup(names[i]);
            if (types && types[i]) pl->types[i] = cco_type_expr_copy(types[i]);
        }
    }
    return pl;
}

void cco_param_list_free(cco_param_list_t *params) {
    if (!params) return;
    for (size_t i = 0; i < params->count; i++) {
        free(params->names[i]);
        cco_type_expr_free(params->types[i]);
    }
    free(params->names);
    free(params->types);
    free(params);
}

/*============================================================================
 * Field init list
 *============================================================================*/

cco_field_init_list_t* cco_field_init_list_new(void) {
    return (cco_field_init_list_t*)calloc(1, sizeof(cco_field_init_list_t));
}

int cco_field_init_list_add(cco_field_init_list_t *list, const char *field_name, cco_expr_t *value_expr) {
    if (!list || !field_name || !value_expr) { cco_expr_free(value_expr); return -1; }
    if (list->count >= list->capacity) {
        size_t new_cap = list->capacity ? list->capacity * 2 : 8;
        char **nn = (char**)realloc(list->field_names, new_cap * sizeof(char*));
        cco_expr_t **ne = (cco_expr_t**)realloc(list->value_exprs, new_cap * sizeof(cco_expr_t*));
        if (!nn || !ne) { free(nn); free(ne); cco_expr_free(value_expr); return -1; }
        list->field_names = nn;
        list->value_exprs = ne;
        list->capacity    = new_cap;
    }
    list->field_names[list->count] = strdup(field_name);
    list->value_exprs[list->count] = value_expr;
    list->count++;
    return 0;
}

void cco_field_init_list_free(cco_field_init_list_t *list) {
    if (!list) return;
    for (size_t i = 0; i < list->count; i++) {
        free(list->field_names[i]);
        cco_expr_free(list->value_exprs[i]);
    }
    free(list->field_names);
    free(list->value_exprs);
    free(list);
}


