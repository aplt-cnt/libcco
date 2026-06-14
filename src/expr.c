#include <cnt/cco_p.h>
#include <stdlib.h>
#include <string.h>

/*============================================================================
 * Expression kinds
 *============================================================================*/
typedef enum {
    CCO_EXPR_LITERAL,
    CCO_EXPR_IDENTIFIER,
    CCO_EXPR_THIS,
    CCO_EXPR_OBJECT,
    CCO_EXPR_ARRAY,
    CCO_EXPR_INSTANTIATE,
    CCO_EXPR_STATIC_CALL,
    CCO_EXPR_FORMAT,
    CCO_EXPR_COMPOUND,
    CCO_EXPR_BINARY,
    CCO_EXPR_UNARY,
    CCO_EXPR_COALESCE,
} cco_expr_kind_t;

struct cco_expr {
    cco_expr_kind_t kind;
    union {
        cco_object_t *literal;
        struct { char *name; } ident;
        struct { char **names; cco_expr_t **exprs; size_t count; } object;
        struct { cco_expr_t **elements; size_t count; } array;
        struct {
            char *template_name;
            cco_expr_t **pos_args;
            size_t pos_count;
            cco_field_init_list_t *named_args;
        } instantiate;
        struct {
            char *template_name;
            char *method_name;
            cco_expr_t **args;
            size_t arg_count;
        } static_call;
        struct { cco_expr_t *arg; } format;
        struct { cco_expr_t *inner; } compound;
        struct { cco_expr_t *lhs; cco_op_t op; cco_expr_t *rhs; } binary;
        struct { cco_op_t op; cco_expr_t *operand; } unary;
        struct { cco_expr_t *left; cco_expr_t *right; } coalesce;
    } data;
};

/*============================================================================
 * Constructors
 *============================================================================*/
cco_expr_t* cco_expr_literal(cco_object_t *val) {
    if (!val) return NULL;
    cco_expr_t *e = (cco_expr_t*)calloc(1, sizeof(cco_expr_t));
    if (!e) { cco_object_free(val); return NULL; }
    e->kind = CCO_EXPR_LITERAL;
    e->data.literal = val;
    return e;
}

cco_expr_t* cco_expr_identifier(const char *name) {
    if (!name) return NULL;
    cco_expr_t *e = (cco_expr_t*)calloc(1, sizeof(cco_expr_t));
    if (!e) return NULL;
    e->kind = CCO_EXPR_IDENTIFIER;
    e->data.ident.name = strdup(name);
    if (!e->data.ident.name) { free(e); return NULL; }
    return e;
}

cco_expr_t* cco_expr_this(void) {
    cco_expr_t *e = (cco_expr_t*)calloc(1, sizeof(cco_expr_t));
    if (e) e->kind = CCO_EXPR_THIS;
    return e;
}

cco_expr_t* cco_expr_object(const char **field_names, cco_expr_t **field_exprs, size_t count) {
    cco_expr_t *e = (cco_expr_t*)calloc(1, sizeof(cco_expr_t));
    if (!e) return NULL;
    e->kind = CCO_EXPR_OBJECT;
    if (count > 0) {
        e->data.object.names = (char**)calloc(count, sizeof(char*));
        e->data.object.exprs = (cco_expr_t**)calloc(count, sizeof(cco_expr_t*));
        if (!e->data.object.names || !e->data.object.exprs) {
            cco_expr_free(e); return NULL;
        }
        for (size_t i = 0; i < count; i++) {
            e->data.object.names[i] = field_names[i] ? strdup(field_names[i]) : NULL;
            e->data.object.exprs[i] = field_exprs[i];
        }
        e->data.object.count = count;
    }
    return e;
}

cco_expr_t* cco_expr_array(cco_expr_t **elements, size_t count) {
    cco_expr_t *e = (cco_expr_t*)calloc(1, sizeof(cco_expr_t));
    if (!e) return NULL;
    e->kind = CCO_EXPR_ARRAY;
    if (count > 0) {
        e->data.array.elements = (cco_expr_t**)calloc(count, sizeof(cco_expr_t*));
        if (!e->data.array.elements) { free(e); return NULL; }
        memcpy(e->data.array.elements, elements, count * sizeof(cco_expr_t*));
        e->data.array.count = count;
    }
    return e;
}

cco_expr_t* cco_expr_instantiate(const char *template_name,
                                 cco_expr_t **positional_args, size_t arg_count,
                                 cco_field_init_list_t *named_args) {
    if (!template_name) return NULL;
    cco_expr_t *e = (cco_expr_t*)calloc(1, sizeof(cco_expr_t));
    if (!e) return NULL;
    e->kind = CCO_EXPR_INSTANTIATE;
    e->data.instantiate.template_name = strdup(template_name);
    if (!e->data.instantiate.template_name) { free(e); return NULL; }
    e->data.instantiate.pos_args  = NULL;
    e->data.instantiate.pos_count = 0;
    e->data.instantiate.named_args = named_args;

    if (arg_count > 0) {
        e->data.instantiate.pos_args = (cco_expr_t**)calloc(arg_count, sizeof(cco_expr_t*));
        if (!e->data.instantiate.pos_args) { cco_expr_free(e); return NULL; }
        memcpy(e->data.instantiate.pos_args, positional_args, arg_count * sizeof(cco_expr_t*));
        e->data.instantiate.pos_count = arg_count;
    }
    return e;
}

cco_expr_t* cco_expr_static_call(const char *template_name, const char *method_name,
                                 cco_expr_t **args, size_t arg_count) {
    if (!template_name || !method_name) return NULL;
    cco_expr_t *e = (cco_expr_t*)calloc(1, sizeof(cco_expr_t));
    if (!e) return NULL;
    e->kind = CCO_EXPR_STATIC_CALL;
    e->data.static_call.template_name = strdup(template_name);
    e->data.static_call.method_name   = strdup(method_name);
    if (!e->data.static_call.template_name || !e->data.static_call.method_name) {
        cco_expr_free(e); return NULL;
    }
    if (arg_count > 0) {
        e->data.static_call.args = (cco_expr_t**)calloc(arg_count, sizeof(cco_expr_t*));
        if (!e->data.static_call.args) { cco_expr_free(e); return NULL; }
        memcpy(e->data.static_call.args, args, arg_count * sizeof(cco_expr_t*));
        e->data.static_call.arg_count = arg_count;
    }
    return e;
}

cco_expr_t* cco_expr_format(cco_expr_t *string_expr) {
    if (!string_expr) return NULL;
    cco_expr_t *e = (cco_expr_t*)calloc(1, sizeof(cco_expr_t));
    if (!e) { cco_expr_free(string_expr); return NULL; }
    e->kind = CCO_EXPR_FORMAT;
    e->data.format.arg = string_expr;
    return e;
}

cco_expr_t* cco_expr_compound(cco_expr_t *inner) {
    if (!inner) return NULL;
    cco_expr_t *e = (cco_expr_t*)calloc(1, sizeof(cco_expr_t));
    if (!e) { cco_expr_free(inner); return NULL; }
    e->kind = CCO_EXPR_COMPOUND;
    e->data.compound.inner = inner;
    return e;
}

cco_expr_t* cco_expr_binary(cco_expr_t *lhs, cco_op_t op, cco_expr_t *rhs) {
    if (!lhs || !rhs) { cco_expr_free(lhs); cco_expr_free(rhs); return NULL; }
    cco_expr_t *e = (cco_expr_t*)calloc(1, sizeof(cco_expr_t));
    if (!e) { cco_expr_free(lhs); cco_expr_free(rhs); return NULL; }
    e->kind = CCO_EXPR_BINARY;
    e->data.binary.lhs = lhs;
    e->data.binary.op  = op;
    e->data.binary.rhs = rhs;
    return e;
}

cco_expr_t* cco_expr_unary(cco_op_t op, cco_expr_t *operand) {
    if (!operand) return NULL;
    cco_expr_t *e = (cco_expr_t*)calloc(1, sizeof(cco_expr_t));
    if (!e) { cco_expr_free(operand); return NULL; }
    e->kind = CCO_EXPR_UNARY;
    e->data.unary.op     = op;
    e->data.unary.operand = operand;
    return e;
}

cco_expr_t* cco_expr_coalesce(cco_expr_t *left, cco_expr_t *right) {
    if (!left || !right) { cco_expr_free(left); cco_expr_free(right); return NULL; }
    cco_expr_t *e = (cco_expr_t*)calloc(1, sizeof(cco_expr_t));
    if (!e) { cco_expr_free(left); cco_expr_free(right); return NULL; }
    e->kind = CCO_EXPR_COALESCE;
    e->data.coalesce.left  = left;
    e->data.coalesce.right = right;
    return e;
}

/*============================================================================
 * Free
 *============================================================================*/
void cco_expr_free(cco_expr_t *expr) {
    if (!expr) return;
    switch (expr->kind) {
    case CCO_EXPR_LITERAL:
        cco_object_free(expr->data.literal);
        break;
    case CCO_EXPR_IDENTIFIER:
        free(expr->data.ident.name);
        break;
    case CCO_EXPR_THIS:
        break;
    case CCO_EXPR_OBJECT:
        for (size_t i = 0; i < expr->data.object.count; i++) {
            free(expr->data.object.names[i]);
            cco_expr_free(expr->data.object.exprs[i]);
        }
        free(expr->data.object.names);
        free(expr->data.object.exprs);
        break;
    case CCO_EXPR_ARRAY:
        for (size_t i = 0; i < expr->data.array.count; i++)
            cco_expr_free(expr->data.array.elements[i]);
        free(expr->data.array.elements);
        break;
    case CCO_EXPR_INSTANTIATE:
        free(expr->data.instantiate.template_name);
        for (size_t i = 0; i < expr->data.instantiate.pos_count; i++)
            cco_expr_free(expr->data.instantiate.pos_args[i]);
        free(expr->data.instantiate.pos_args);
        cco_field_init_list_free(expr->data.instantiate.named_args);
        break;
    case CCO_EXPR_STATIC_CALL:
        free(expr->data.static_call.template_name);
        free(expr->data.static_call.method_name);
        for (size_t i = 0; i < expr->data.static_call.arg_count; i++)
            cco_expr_free(expr->data.static_call.args[i]);
        free(expr->data.static_call.args);
        break;
    case CCO_EXPR_FORMAT:
        cco_expr_free(expr->data.format.arg);
        break;
    case CCO_EXPR_COMPOUND:
        cco_expr_free(expr->data.compound.inner);
        break;
    case CCO_EXPR_BINARY:
        cco_expr_free(expr->data.binary.lhs);
        cco_expr_free(expr->data.binary.rhs);
        break;
    case CCO_EXPR_UNARY:
        cco_expr_free(expr->data.unary.operand);
        break;
    case CCO_EXPR_COALESCE:
        cco_expr_free(expr->data.coalesce.left);
        cco_expr_free(expr->data.coalesce.right);
        break;
    }
    free(expr);
}

/*============================================================================
 * Evaluate — helpers for inheritance, constructors
 *============================================================================*/
static cco_object_t* eval_expr(const cco_expr_t *expr, cco_symbol_table_t *st,
                               cco_object_t *self, cco_object_t *locals);
static cco_object_t* eval_stmt_seq(const cco_method_body_t *body,
                                   cco_symbol_table_t *st,
                                   cco_object_t *self, cco_object_t *locals);

/* Collect all fields (own + inherited) into a flat array.
   Returns number of fields collected, or 0 on error.
   The caller must not free the name pointers (they point into templates). */
typedef struct {
    const char **names;
    int         *has_default;
    size_t       count;
} flat_field_list_t;

/* Recursively collect fields: parent first, then child */
static size_t collect_fields_r(cco_symbol_table_t *st, const char *name,
                               const char **names, int *has_default, size_t offset) {
    cco_template_t *tmpl = cco_symbol_table_lookup_template(st, name);
    if (!tmpl) return offset;

    /* First collect parent fields */
    const char *pname = cco_template_get_parent_name(tmpl);
    if (pname) offset = collect_fields_r(st, pname, names, has_default, offset);

    /* Then collect own fields */
    size_t n = cco_template_field_count(tmpl);
    for (size_t i = 0; i < n; i++) {
        names[offset] = cco_template_field_name(tmpl, i);
        has_default[offset] = (cco_template_field_default(tmpl, i) != NULL);
        offset++;
    }
    return offset;
}

static flat_field_list_t collect_fields(cco_symbol_table_t *st, const char *template_name) {
    flat_field_list_t result = { NULL, NULL, 0 };

    /* First count total fields across hierarchy */
    size_t total = 0;
    const char *cur = template_name;
    while (cur) {
        cco_template_t *t = cco_symbol_table_lookup_template(st, cur);
        if (!t) break;
        total += cco_template_field_count(t);
        cur = cco_template_get_parent_name(t);
    }
    if (total == 0) return result;

    result.names = (const char**)malloc(total * sizeof(const char*));
    result.has_default = (int*)malloc(total * sizeof(int));
    if (!result.names || !result.has_default) {
        free(result.names); free(result.has_default);
        result.names = NULL; result.has_default = NULL;
        return result;
    }

    size_t written = collect_fields_r(st, template_name, result.names, result.has_default, 0);
    result.count = written;
    return result;
}

static void free_flat_fields(flat_field_list_t *f) {
    free(f->names);
    free(f->has_default);
    f->names = NULL;
    f->has_default = NULL;
    f->count = 0;
}

/* Try to evaluate a method body (statements + final expression) */
static cco_object_t* eval_method_body(const cco_method_body_t *body,
                                      cco_symbol_table_t *st,
                                      cco_object_t *self, cco_object_t *locals) {
    if (!body) return cco_none_new();

    /* Evaluate statements */
    /* We can't access body internals from here without struct definition.
       For now, we handle core cases in eval_stmt_seq which has access. */
    return eval_stmt_seq(body, st, self, locals);
}

/*============================================================================
 * eval_instantiate: handles positional & named instantiation,
 *                   inheritance, constructors, field defaults
 *============================================================================*/
static cco_object_t* eval_instantiate(const cco_expr_t *e, cco_symbol_table_t *st,
                                      cco_object_t *locals) {
    const char *name = e->data.instantiate.template_name;

    cco_template_t *tmpl = cco_symbol_table_lookup_template(st, name);
    if (!tmpl) { CCO_ERR("unknown template '%s'", name); return NULL; }

    /* Collect all fields (own + inherited) */
    flat_field_list_t fields = collect_fields(st, name);
    if (fields.count == 0 && e->data.instantiate.pos_count == 0 &&
        !e->data.instantiate.named_args) {
        /* No fields but no args either — create empty instance */
    }

    /* Handle named instantiation: #T(.x: val, .y: val) */
    if (e->data.instantiate.named_args) {
        cco_object_t *inst = cco_template_instance_new(name);
        if (!inst) { free_flat_fields(&fields); return NULL; }

        /* For each field in named_args, evaluate and set */
        for (size_t i = 0; i < e->data.instantiate.named_args->count; i++) {
            cco_object_t *v = eval_expr(e->data.instantiate.named_args->value_exprs[i],
                                        st, NULL, locals);
            if (!v) { cco_object_free(inst); free_flat_fields(&fields); return NULL; }
            cco_object_set(inst, e->data.instantiate.named_args->field_names[i], v);
        }

        /* Apply defaults for missing fields */
        for (size_t i = 0; i < fields.count; i++) {
            if (e->data.instantiate.named_args->field_names) {
                int found = 0;
                for (size_t j = 0; j < e->data.instantiate.named_args->count; j++) {
                    if (strcmp(e->data.instantiate.named_args->field_names[j], fields.names[i]) == 0)
                    { found = 1; break; }
                }
                if (!found && fields.has_default[i]) {
                    /* Find the template that owns this field to get the default expr */
                    const char *cur = name;
                    while (cur) {
                        cco_template_t *t = cco_symbol_table_lookup_template(st, cur);
                        if (!t) break;
                        for (size_t k = 0; k < cco_template_field_count(t); k++) {
                            if (strcmp(cco_template_field_name(t, k), fields.names[i]) == 0) {
                                cco_expr_t *def = cco_template_field_default(t, k);
                                if (def) {
                                    cco_object_t *dv = eval_expr(def, st, NULL, locals);
                                    if (dv) cco_object_set(inst, fields.names[i], dv);
                                    else cco_object_free(dv);
                                }
                                break;
                            }
                        }
                        cur = cco_template_get_parent_name(t);
                    }
                }
            }
        }

        free_flat_fields(&fields);
        return inst;
    }

    /* Positional instantiation: #T(arg1, arg2, ...) */
    size_t arg_count = e->data.instantiate.pos_count;
    cco_object_t **arg_vals = NULL;
    if (arg_count > 0) {
        arg_vals = (cco_object_t**)calloc(arg_count, sizeof(cco_object_t*));
        if (!arg_vals) { free_flat_fields(&fields); return NULL; }
        for (size_t i = 0; i < arg_count; i++) {
            arg_vals[i] = eval_expr(e->data.instantiate.pos_args[i], st, NULL, locals);
            if (!arg_vals[i]) {
                for (size_t j = 0; j < i; j++) cco_object_free(arg_vals[j]);
                free(arg_vals); free_flat_fields(&fields);
                return NULL;
            }
        }
    }

    cco_object_t *inst = cco_template_instance_new(name);
    if (!inst) {
        for (size_t i = 0; i < arg_count; i++) cco_object_free(arg_vals[i]);
        free(arg_vals); free_flat_fields(&fields);
        return NULL;
    }

    /* Try to find an explicit constructor matching arg count */
    cco_constructor_t *match_ctor = NULL;
    size_t ctor_count = cco_template_constructor_count(tmpl);
    for (size_t ci = 0; ci < ctor_count; ci++) {
        cco_constructor_t *c = cco_template_get_constructor(tmpl, ci);
        if (c && !cco_constructor_is_default(c) &&
            cco_constructor_param_count(c) == arg_count) {
            match_ctor = c;
            break;
        }
    }

    if (match_ctor) {
        /* Build locals from param names → arg values */
        cco_object_t *ctor_locals = cco_object_new();
        for (size_t i = 0; i < arg_count; i++) {
            const char *pname = cco_constructor_param_name(match_ctor, i);
            if (pname && arg_vals[i]) {
                cco_object_set(ctor_locals, pname, arg_vals[i]);
            }
        }

        /* Evaluate constructor body with self=inst, locals=param_map */
        cco_method_body_t *body = cco_constructor_get_body(match_ctor);
        if (body) {
            cco_object_t *body_result = eval_stmt_seq(body, st, inst, ctor_locals);
            cco_object_free(body_result); /* discard, we keep inst */
        }

        cco_object_free(ctor_locals);
        free(arg_vals);
        free_flat_fields(&fields);
        return inst;
    }

    /* Default constructor: set fields from positional args in field order */
    size_t set_count = arg_count < fields.count ? arg_count : fields.count;
    for (size_t i = 0; i < set_count; i++) {
        if (fields.names[i])
            cco_object_set(inst, fields.names[i], arg_vals[i]);
        else
            cco_object_free(arg_vals[i]);
    }
    free(arg_vals);

    /* Evaluate defaults for any missing fields that have defaults */
    if (arg_count < fields.count) {
        for (size_t i = arg_count; i < fields.count; i++) {
            if (!fields.has_default[i]) continue;
            const char *cur = name;
            while (cur) {
                cco_template_t *t = cco_symbol_table_lookup_template(st, cur);
                if (!t) break;
                for (size_t k = 0; k < cco_template_field_count(t); k++) {
                    if (strcmp(cco_template_field_name(t, k), fields.names[i]) == 0) {
                        cco_expr_t *def = cco_template_field_default(t, k);
                        if (def) {
                            cco_object_t *dv = eval_expr(def, st, NULL, locals);
                            if (dv) cco_object_set(inst, fields.names[i], dv);
                            else cco_object_free(dv);
                        }
                        break;
                    }
                }
                cur = cco_template_get_parent_name(t);
            }
        }
    }

    free_flat_fields(&fields);
    return inst;
}

/*============================================================================
 * Numeric coercion helpers
 *============================================================================*/
static int coerce_numbers(cco_object_t *a, cco_object_t *b,
                          long long *ai, double *af, int *a_is_float,
                          long long *bi, double *bf, int *b_is_float) {
    if (!a || !b) return 0;
    int af_ = (a->type == CCO_TYPE_FLOAT);
    int bf_ = (b->type == CCO_TYPE_FLOAT);
    *a_is_float = af_;
    *b_is_float = bf_;
    if (af_ || bf_) {
        *af = af_ ? a->data.float_val : (double)a->data.int_val;
        *bf = bf_ ? b->data.float_val : (double)b->data.int_val;
    } else {
        *ai = a->data.int_val;
        *bi = b->data.int_val;
    }
    return 1;
}

static cco_object_t* eval_binary(const cco_expr_t *e, cco_symbol_table_t *st,
                                  cco_object_t *self, cco_object_t *locals) {
    cco_object_t *l = eval_expr(e->data.binary.lhs, st, self, locals);
    if (!l) return NULL;
    cco_object_t *r = eval_expr(e->data.binary.rhs, st, self, locals);
    if (!r) { cco_object_free(l); return NULL; }

    cco_object_t *result = NULL;

    switch (e->data.binary.op) {
    /* Arithmetic */
    case CCO_OP_ADD: case CCO_OP_SUB: case CCO_OP_MUL: case CCO_OP_DIV: {
        if ((l->type != CCO_TYPE_INT && l->type != CCO_TYPE_FLOAT) ||
            (r->type != CCO_TYPE_INT && r->type != CCO_TYPE_FLOAT)) {
            CCO_ERR("arithmetic on non-numeric types");
            break;
        }
        long long li, ri; double lf, rf; int lfi, rfi;
        if (!coerce_numbers(l, r, &li, &lf, &lfi, &ri, &rf, &rfi)) break;

        if (lfi || rfi) {
            double v = 0;
            switch (e->data.binary.op) {
            case CCO_OP_ADD: v = lf + rf; break;
            case CCO_OP_SUB: v = lf - rf; break;
            case CCO_OP_MUL: v = lf * rf; break;
            case CCO_OP_DIV:
                if (rf == 0.0) { CCO_ERR("division by zero"); break; }
                v = lf / rf; break;
            default: break;
            }
            result = cco_float_new(v);
        } else {
            long long v = 0;
            switch (e->data.binary.op) {
            case CCO_OP_ADD: v = li + ri; break;
            case CCO_OP_SUB: v = li - ri; break;
            case CCO_OP_MUL: v = li * ri; break;
            case CCO_OP_DIV:
                if (ri == 0) { CCO_ERR("division by zero"); break; }
                v = li / ri; break;
            default: break;
            }
            result = cco_int_new(v);
        }
        break;
    }

    /* Comparisons */
    case CCO_OP_EQ: case CCO_OP_NE:
    case CCO_OP_LT: case CCO_OP_GT: case CCO_OP_LE: case CCO_OP_GE: {
        int cmp = 0;
        if (l->type == CCO_TYPE_INT && r->type == CCO_TYPE_INT) {
            long long a = l->data.int_val, b = r->data.int_val;
            if (a < b) cmp = -1; else if (a > b) cmp = 1;
        } else if (l->type == CCO_TYPE_FLOAT || r->type == CCO_TYPE_FLOAT) {
            double a = (l->type == CCO_TYPE_FLOAT) ? l->data.float_val : (double)l->data.int_val;
            double b = (r->type == CCO_TYPE_FLOAT) ? r->data.float_val : (double)r->data.int_val;
            if (a < b) cmp = -1; else if (a > b) cmp = 1;
        } else if (l->type == CCO_TYPE_STRING && r->type == CCO_TYPE_STRING) {
            int c = strcmp(l->data.string.data ? l->data.string.data : "",
                          r->data.string.data ? r->data.string.data : "");
            if (c < 0) cmp = -1; else if (c > 0) cmp = 1;
        } else if (l->type == CCO_TYPE_BOOL && r->type == CCO_TYPE_BOOL) {
            int a = l->data.bool_val, b = r->data.bool_val;
            if (a < b) cmp = -1; else if (a > b) cmp = 1;
        } else {
            CCO_ERR("comparison of incompatible types");
            break;
        }
        int val = 0;
        switch (e->data.binary.op) {
        case CCO_OP_EQ: val = (cmp == 0); break;
        case CCO_OP_NE: val = (cmp != 0); break;
        case CCO_OP_LT: val = (cmp < 0);  break;
        case CCO_OP_GT: val = (cmp > 0);  break;
        case CCO_OP_LE: val = (cmp <= 0); break;
        case CCO_OP_GE: val = (cmp >= 0); break;
        default: break;
        }
        result = cco_bool_new(val);
        break;
    }

    /* Logical */
    case CCO_OP_AND: {
        int lv = (l->type == CCO_TYPE_BOOL && l->data.bool_val);
        result = cco_bool_new(lv && (r->type == CCO_TYPE_BOOL && r->data.bool_val));
        break;
    }
    case CCO_OP_OR: {
        int lv = (l->type == CCO_TYPE_BOOL && l->data.bool_val);
        result = cco_bool_new(lv || (r->type == CCO_TYPE_BOOL && r->data.bool_val));
        break;
    }
    default:
        CCO_ERR("unknown binary op %d", (int)e->data.binary.op);
        break;
    }

    cco_object_free(l);
    cco_object_free(r);
    return result;
}

static cco_object_t* eval_unary(const cco_expr_t *e, cco_symbol_table_t *st,
                                 cco_object_t *self, cco_object_t *locals) {
    cco_object_t *op = eval_expr(e->data.unary.operand, st, self, locals);
    if (!op) return NULL;

    cco_object_t *result = NULL;
    switch (e->data.unary.op) {
    case CCO_OP_NOT:
        result = cco_bool_new(!(op->type == CCO_TYPE_BOOL && op->data.bool_val));
        break;
    default:
        CCO_ERR("unknown unary op %d", (int)e->data.unary.op);
        break;
    }
    cco_object_free(op);
    return result;
}

static cco_object_t* eval_coalesce(const cco_expr_t *e, cco_symbol_table_t *st,
                                    cco_object_t *self, cco_object_t *locals) {
    cco_object_t *l = eval_expr(e->data.coalesce.left, st, self, locals);
    if (!l) return NULL;
    if (l->type != CCO_TYPE_NONE) return l; /* caller owns result */
    cco_object_free(l);
    return eval_expr(e->data.coalesce.right, st, self, locals);
}

static cco_object_t* eval_format(const cco_expr_t *e, cco_symbol_table_t *st,
                                  cco_object_t *self, cco_object_t *locals) {
    cco_object_t *arg = eval_expr(e->data.format.arg, st, self, locals);
    if (!arg || arg->type != CCO_TYPE_STRING) {
        cco_object_free(arg);
        CCO_ERR("$format requires a string argument");
        return NULL;
    }
    const char *text = arg->data.string.data ? arg->data.string.data : "";
    /* Parse the string as a CCO value */
    cco_object_t *parsed = cco_parse(text);
    cco_object_free(arg);
    return parsed; /* NULL if parse failed (error already set by cco_parse) */
}

static cco_object_t* eval_static_call(const cco_expr_t *e, cco_symbol_table_t *st,
                                       cco_object_t *locals) {
    const char *tname = e->data.static_call.template_name;
    const char *mname = e->data.static_call.method_name;

    cco_template_t *tmpl = cco_symbol_table_lookup_template(st, tname);
    if (!tmpl) { CCO_ERR("unknown template '%s'", tname); return NULL; }

    /* Evaluate args */
    size_t n = e->data.static_call.arg_count;
    cco_object_t **arg_vals = NULL;
    if (n > 0) {
        arg_vals = (cco_object_t**)calloc(n, sizeof(cco_object_t*));
        if (!arg_vals) return NULL;
        for (size_t i = 0; i < n; i++) {
            arg_vals[i] = eval_expr(e->data.static_call.args[i], st, NULL, locals);
            if (!arg_vals[i]) {
                for (size_t j = 0; j < i; j++) cco_object_free(arg_vals[j]);
                free(arg_vals); return NULL;
            }
        }
    }

    /* Build locals for method body: params → args */
    /* For now, if there's no method body, just return None */
    cco_object_t *result = cco_none_new();
    free(arg_vals);
    return result;
}

/*============================================================================
 * Statement evaluator
 *============================================================================*/
static cco_object_t* eval_stmt_seq(const cco_method_body_t *body,
                                   cco_symbol_table_t *st,
                                   cco_object_t *self, cco_object_t *locals) {
    if (!body) return cco_none_new();
    if (!locals) {
        locals = cco_object_new();
    }

    for (size_t i = 0; i < body->stmt_count; i++) {
        cco_stmt_t *s = body->stmts[i];
        switch (s->kind) {
        case CCO_STMT_BIND: {
            cco_object_t *v = eval_expr(s->data.bind.value, st, self, locals);
            if (!v) return NULL;
            cco_object_set(locals, s->data.bind.name, v);
            break;
        }
        case CCO_STMT_ASSIGN: {
            cco_object_t *v = eval_expr(s->data.assign.value, st, self, locals);
            if (!v) return NULL;
            if (self) cco_object_set(self, s->data.assign.field_name, v);
            else { cco_object_free(v); return NULL; }
            break;
        }
        case CCO_STMT_RETURN: {
            cco_object_t *v = eval_expr(s->data.return_stmt.value, st, self, locals);
            return v; /* early return */
        }
        case CCO_STMT_EXPR: {
            cco_object_t *v = eval_expr(s->data.expr_stmt.expr, st, self, locals);
            cco_object_free(v); /* discard */
            break;
        }
        }
    }

    /* Evaluate final expression */
    if (body->final_expr)
        return eval_expr(body->final_expr, st, self, locals);
    return cco_none_new();
}

static cco_object_t* eval_expr(const cco_expr_t *expr, cco_symbol_table_t *st,
                               cco_object_t *self, cco_object_t *locals) {
    if (!expr) { CCO_ERR("eval_expr: NULL expression"); return NULL; }

    switch (expr->kind) {
    case CCO_EXPR_LITERAL:
        return cco_object_copy(expr->data.literal);

    case CCO_EXPR_IDENTIFIER: {
        const char *name = expr->data.ident.name;
        if (locals) {
            cco_object_t *v = cco_object_get(locals, name);
            if (v) return cco_object_copy(v);
        }
        if (self) {
            cco_object_t *v = cco_object_get(self, name);
            if (v) return cco_object_copy(v);
        }
        CCO_ERR("undefined identifier '%s'", name);
        return NULL;
    }

    case CCO_EXPR_THIS:
        if (self) return cco_object_copy(self);
        CCO_ERR("'this' used outside instance context");
        return NULL;

    case CCO_EXPR_OBJECT: {
        cco_object_t *obj = cco_object_new();
        if (!obj) return NULL;
        for (size_t i = 0; i < expr->data.object.count; i++) {
            cco_object_t *v = eval_expr(expr->data.object.exprs[i], st, self, locals);
            if (!v) { cco_object_free(obj); return NULL; }
            cco_object_set(obj, expr->data.object.names[i], v);
        }
        return obj;
    }

    case CCO_EXPR_ARRAY: {
        cco_array_t *arr = cco_array_new();
        if (!arr) return NULL;
        for (size_t i = 0; i < expr->data.array.count; i++) {
            cco_object_t *v = eval_expr(expr->data.array.elements[i], st, self, locals);
            if (!v) { cco_array_free(arr); return NULL; }
            cco_array_add(arr, v);
        }
        return cco_array_wrap(arr);
    }

    case CCO_EXPR_INSTANTIATE:
        return eval_instantiate(expr, st, locals);

    case CCO_EXPR_STATIC_CALL:
        return eval_static_call(expr, st, locals);

    case CCO_EXPR_FORMAT:
        return eval_format(expr, st, self, locals);

    case CCO_EXPR_COMPOUND:
        return eval_expr(expr->data.compound.inner, st, self, locals);

    case CCO_EXPR_BINARY:
        return eval_binary(expr, st, self, locals);

    case CCO_EXPR_UNARY:
        return eval_unary(expr, st, self, locals);

    case CCO_EXPR_COALESCE:
        return eval_coalesce(expr, st, self, locals);

    default:
        CCO_ERR("expression kind %d not yet implemented", (int)expr->kind);
        return NULL;
    }
}

/*============================================================================
 * Public evaluator API
 *============================================================================*/
cco_object_t* cco_expr_eval(const cco_expr_t *expr, cco_symbol_table_t *st,
                            cco_object_t *self, cco_object_t *locals) {
    return eval_expr(expr, st, self, locals);
}

cco_object_t* cco_instantiate(cco_symbol_table_t *st, const char *template_name,
                              cco_object_t **args, size_t arg_count) {
    /* Build expression tree to reuse eval machinery */
    cco_expr_t **exprs = NULL;
    if (arg_count > 0) {
        exprs = (cco_expr_t**)calloc(arg_count, sizeof(cco_expr_t*));
        if (!exprs) return NULL;
        for (size_t i = 0; i < arg_count; i++) {
            exprs[i] = cco_expr_literal(args[i]);
            if (!exprs[i]) {
                for (size_t j = 0; j < i; j++) cco_expr_free(exprs[j]);
                free(exprs);
                return NULL;
            }
        }
    }

    cco_expr_t *inst_expr = cco_expr_instantiate(template_name, exprs, arg_count, NULL);
    free(exprs); /* expressions are owned by inst_expr now */

    if (!inst_expr) return NULL;

    cco_object_t *result = cco_expr_eval(inst_expr, st, NULL, NULL);
    cco_expr_free(inst_expr);
    return result;
}

cco_object_t* cco_call_static(cco_symbol_table_t *st, const char *template_name,
                              const char *method_name, cco_object_t **args, size_t arg_count) {
    /* Build expression tree */
    cco_expr_t **exprs = NULL;
    if (arg_count > 0) {
        exprs = (cco_expr_t**)calloc(arg_count, sizeof(cco_expr_t*));
        if (!exprs) return NULL;
        for (size_t i = 0; i < arg_count; i++) {
            exprs[i] = cco_expr_literal(args[i]);
            if (!exprs[i]) {
                for (size_t j = 0; j < i; j++) cco_expr_free(exprs[j]);
                free(exprs); return NULL;
            }
        }
    }
    cco_expr_t *call = cco_expr_static_call(template_name, method_name, exprs, arg_count);
    free(exprs);
    if (!call) return NULL;
    cco_object_t *result = cco_expr_eval(call, st, NULL, NULL);
    cco_expr_free(call);
    return result;
}
