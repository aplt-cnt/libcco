#include <cnt/cco_p.h>
#include <stdlib.h>
#include <string.h>

#define INITIAL_CAPACITY 64

/*============================================================================
 * Symbol entry
 *============================================================================*/
typedef enum {
    CCO_ENTRY_TYPEDEF,
    CCO_ENTRY_ENUM,
    CCO_ENTRY_TEMPLATE
} entry_kind_t;

typedef struct sym_entry {
    char           *name;
    entry_kind_t    kind;
    union {
        cco_type_expr_t *alias_type;
        struct {
            char  **values;
            size_t  count;
        } enum_data;
        cco_template_t *templ;
    } data;
    struct sym_entry *next;  /* hash chain */
} sym_entry_t;

static sym_entry_t* entry_new(const char *name, entry_kind_t kind) {
    sym_entry_t *e = (sym_entry_t*)calloc(1, sizeof(sym_entry_t));
    if (!e) return NULL;
    e->name = strdup(name);
    e->kind = kind;
    return e;
}

static void entry_free(sym_entry_t *e) {
    if (!e) return;
    free(e->name);
    switch (e->kind) {
    case CCO_ENTRY_TYPEDEF:
        cco_type_expr_free(e->data.alias_type);
        break;
    case CCO_ENTRY_ENUM:
        for (size_t i = 0; i < e->data.enum_data.count; i++)
            free(e->data.enum_data.values[i]);
        free(e->data.enum_data.values);
        break;
    case CCO_ENTRY_TEMPLATE:
        cco_template_free(e->data.templ);
        break;
    }
    free(e);
}

/*============================================================================
 * Hash function
 *============================================================================*/
static size_t hash_str(const char *s, size_t cap) {
    size_t h = 5381;
    while (*s)
        h = ((h << 5) + h) + (unsigned char)*s++;
    return h % cap;
}

/*============================================================================
 * Symbol table
 *============================================================================*/
struct cco_symbol_table {
    sym_entry_t **buckets;
    size_t        count;
    size_t        capacity;
    /* Ordered list for serialization */
    char        **order_names;
    size_t        order_count;
    size_t        order_capacity;
};

cco_symbol_table_t* cco_symbol_table_new(void) {
    cco_symbol_table_t *st = (cco_symbol_table_t*)calloc(1, sizeof(cco_symbol_table_t));
    if (!st) return NULL;
    st->buckets = (sym_entry_t**)calloc(INITIAL_CAPACITY, sizeof(sym_entry_t*));
    if (!st->buckets) { free(st); return NULL; }
    st->capacity = INITIAL_CAPACITY;
    st->count    = 0;
    return st;
}

void cco_symbol_table_free(cco_symbol_table_t *st) {
    if (!st) return;
    for (size_t i = 0; i < st->capacity; i++) {
        sym_entry_t *e = st->buckets[i];
        while (e) {
            sym_entry_t *next = e->next;
            entry_free(e);
            e = next;
        }
    }
    free(st->buckets);
    for (size_t i = 0; i < st->order_count; i++) free(st->order_names[i]);
    free(st->order_names);
    free(st);
}

/* Track order for serialization */
static int add_order(cco_symbol_table_t *st, const char *name) {
    if (st->order_count >= st->order_capacity) {
        size_t nc = st->order_capacity ? st->order_capacity * 2 : 16;
        char **nn = (char**)realloc(st->order_names, nc * sizeof(char*));
        if (!nn) return -1;
        st->order_names = nn;
        st->order_capacity = nc;
    }
    st->order_names[st->order_count++] = strdup(name);
    return 0;
}

static sym_entry_t* find_entry(const cco_symbol_table_t *st, const char *name) {
    if (!st || !name) return NULL;
    size_t idx = hash_str(name, st->capacity);
    sym_entry_t *e = st->buckets[idx];
    while (e) {
        if (strcmp(e->name, name) == 0) return e;
        e = e->next;
    }
    return NULL;
}

int cco_symbol_table_add_typedef(cco_symbol_table_t *st, const char *name, cco_type_expr_t *type) {
    if (!st || !name || !type) { cco_type_expr_free(type); return -1; }
    if (find_entry(st, name)) {
        cco_type_expr_free(type);
        return CCO_ERR("symbol '%s' already exists", name);
    }

    sym_entry_t *e = entry_new(name, CCO_ENTRY_TYPEDEF);
    if (!e) { cco_type_expr_free(type); return -1; }
    e->data.alias_type = type;

    size_t idx = hash_str(name, st->capacity);
    e->next = st->buckets[idx];
    st->buckets[idx] = e;
    st->count++;
    add_order(st, name);
    return 0;
}

int cco_symbol_table_add_enum(cco_symbol_table_t *st, const char *name, const char **values) {
    if (!st || !name || !values) return -1;
    if (find_entry(st, name))
        return CCO_ERR("symbol '%s' already exists", name);

    /* Count values */
    size_t count = 0;
    while (values[count]) count++;

    sym_entry_t *e = entry_new(name, CCO_ENTRY_ENUM);
    if (!e) return -1;

    e->data.enum_data.values = (char**)calloc(count, sizeof(char*));
    if (!e->data.enum_data.values && count > 0) { entry_free(e); return -1; }
    for (size_t i = 0; i < count; i++) {
        e->data.enum_data.values[i] = strdup(values[i]);
        if (!e->data.enum_data.values[i]) {
            for (size_t j = 0; j < i; j++) free(e->data.enum_data.values[j]);
            free(e->data.enum_data.values);
            entry_free(e);
            return -1;
        }
    }
    e->data.enum_data.count = count;

    size_t idx = hash_str(name, st->capacity);
    e->next = st->buckets[idx];
    st->buckets[idx] = e;
    st->count++;
    add_order(st, name);
    return 0;
}

int cco_symbol_table_add_template(cco_symbol_table_t *st, cco_template_t *tmpl) {
    if (!st || !tmpl) return -1;
    const char *name = cco_template_get_name(tmpl);
    if (!name) return CCO_ERR("template has no name");
    if (find_entry(st, name)) {
        cco_template_free(tmpl);
        return CCO_ERR("symbol '%s' already exists", name);
    }
    sym_entry_t *e = entry_new(name, CCO_ENTRY_TEMPLATE);
    if (!e) { cco_template_free(tmpl); return -1; }
    e->data.templ = tmpl;
    size_t idx = hash_str(name, st->capacity);
    e->next = st->buckets[idx];
    st->buckets[idx] = e;
    st->count++;
    add_order(st, name);
    return 0;
}

/* cco_template_get_name is implemented in src/template.c */

/*============================================================================
 * Public API: lookup functions
 *============================================================================*/
cco_type_expr_t* cco_symbol_table_lookup_type(const cco_symbol_table_t *st, const char *name) {
    sym_entry_t *e = find_entry(st, name);
    if (!e || e->kind != CCO_ENTRY_TYPEDEF) return NULL;
    return e->data.alias_type;
}

int cco_symbol_table_has_enum_value(const cco_symbol_table_t *st, const char *enum_name, const char *value_name) {
    sym_entry_t *e = find_entry(st, enum_name);
    if (!e || e->kind != CCO_ENTRY_ENUM) return 0;
    for (size_t i = 0; i < e->data.enum_data.count; i++) {
        if (strcmp(e->data.enum_data.values[i], value_name) == 0)
            return 1;
    }
    return 0;
}

cco_template_t* cco_symbol_table_lookup_template(const cco_symbol_table_t *st, const char *name) {
    sym_entry_t *e = find_entry(st, name);
    if (!e || e->kind != CCO_ENTRY_TEMPLATE) return NULL;
    return e->data.templ;
}

/*============================================================================
 * Iteration accessors (for serialization)
 *============================================================================*/
size_t cco_symtab_get_count(const cco_symbol_table_t *st) {
    return st ? st->order_count : 0;
}

const char* cco_symtab_get_name(const cco_symbol_table_t *st, size_t idx) {
    if (!st || idx >= st->order_count) return NULL;
    return st->order_names[idx];
}

cco_type_expr_t* cco_symtab_get_typedef(const cco_symbol_table_t *st, const char *name) {
    sym_entry_t *e = find_entry(st, name);
    if (!e || e->kind != CCO_ENTRY_TYPEDEF) return NULL;
    return e->data.alias_type;
}

int cco_symtab_get_enum_values(const cco_symbol_table_t *st, const char *name,
                                const char ***out_values, size_t *out_count) {
    sym_entry_t *e = find_entry(st, name);
    if (!e || e->kind != CCO_ENTRY_ENUM) return 0;
    if (out_values) *out_values = (const char**)e->data.enum_data.values;
    if (out_count)  *out_count  = e->data.enum_data.count;
    return 1;
}

cco_template_t* cco_symtab_get_template(const cco_symbol_table_t *st, const char *name) {
    sym_entry_t *e = find_entry(st, name);
    if (!e || e->kind != CCO_ENTRY_TEMPLATE) return NULL;
    return e->data.templ;
}
