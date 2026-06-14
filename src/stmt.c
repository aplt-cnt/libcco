#include <cnt/cco_p.h>
#include <stdlib.h>
#include <string.h>

/*============================================================================
 * Statement implementation
 *============================================================================*/

cco_stmt_t* cco_stmt_bind(const char *name, cco_type_expr_t *type, cco_expr_t *value) {
    if (!name) { cco_type_expr_free(type); cco_expr_free(value); return NULL; }
    cco_stmt_t *s = (cco_stmt_t*)calloc(1, sizeof(cco_stmt_t));
    if (!s) { cco_type_expr_free(type); cco_expr_free(value); return NULL; }
    s->kind          = CCO_STMT_BIND;
    s->data.bind.name  = strdup(name);
    s->data.bind.type  = type;
    s->data.bind.value = value;
    if (!s->data.bind.name) { free(s); cco_type_expr_free(type); cco_expr_free(value); return NULL; }
    return s;
}

cco_stmt_t* cco_stmt_assign(const char *field_name, cco_expr_t *value) {
    if (!field_name) { cco_expr_free(value); return NULL; }
    cco_stmt_t *s = (cco_stmt_t*)calloc(1, sizeof(cco_stmt_t));
    if (!s) { cco_expr_free(value); return NULL; }
    s->kind                = CCO_STMT_ASSIGN;
    s->data.assign.field_name = strdup(field_name);
    s->data.assign.value      = value;
    if (!s->data.assign.field_name) { free(s); cco_expr_free(value); return NULL; }
    return s;
}

cco_stmt_t* cco_stmt_return(cco_type_expr_t *type, cco_expr_t *value) {
    cco_stmt_t *s = (cco_stmt_t*)calloc(1, sizeof(cco_stmt_t));
    if (!s) { cco_type_expr_free(type); cco_expr_free(value); return NULL; }
    s->kind                = CCO_STMT_RETURN;
    s->data.return_stmt.type  = type;
    s->data.return_stmt.value = value;
    return s;
}

cco_stmt_t* cco_stmt_expr(cco_expr_t *expr) {
    if (!expr) return NULL;
    cco_stmt_t *s = (cco_stmt_t*)calloc(1, sizeof(cco_stmt_t));
    if (!s) { cco_expr_free(expr); return NULL; }
    s->kind            = CCO_STMT_EXPR;
    s->data.expr_stmt.expr = expr;
    return s;
}

void cco_stmt_free(cco_stmt_t *stmt) {
    if (!stmt) return;
    switch (stmt->kind) {
    case CCO_STMT_BIND:
        free(stmt->data.bind.name);
        cco_type_expr_free(stmt->data.bind.type);
        cco_expr_free(stmt->data.bind.value);
        break;
    case CCO_STMT_ASSIGN:
        free(stmt->data.assign.field_name);
        cco_expr_free(stmt->data.assign.value);
        break;
    case CCO_STMT_RETURN:
        cco_type_expr_free(stmt->data.return_stmt.type);
        cco_expr_free(stmt->data.return_stmt.value);
        break;
    case CCO_STMT_EXPR:
        cco_expr_free(stmt->data.expr_stmt.expr);
        break;
    }
    free(stmt);
}

/*============================================================================
 * Method body implementation
 *============================================================================*/

cco_method_body_t* cco_method_body_new(void) {
    cco_method_body_t *mb = (cco_method_body_t*)calloc(1, sizeof(cco_method_body_t));
    return mb;
}

int cco_method_body_add_stmt(cco_method_body_t *body, cco_stmt_t *stmt) {
    if (!body || !stmt) return CCO_ERR("cco_method_body_add_stmt: NULL argument");
    if (body->stmt_count >= body->stmt_capacity) {
        size_t new_cap = body->stmt_capacity ? body->stmt_capacity * 2 : 4;
        cco_stmt_t **p = (cco_stmt_t**)realloc(body->stmts, new_cap * sizeof(cco_stmt_t*));
        if (!p) { cco_stmt_free(stmt); return CCO_ERR("cco_method_body_add_stmt: out of memory"); }
        body->stmts      = p;
        body->stmt_capacity = new_cap;
    }
    body->stmts[body->stmt_count++] = stmt;
    return 0;
}

void cco_method_body_set_final_expr(cco_method_body_t *body, cco_expr_t *expr) {
    if (!body) return;
    cco_expr_free(body->final_expr);
    body->final_expr = expr;
}

void cco_method_body_free(cco_method_body_t *body) {
    if (!body) return;
    for (size_t i = 0; i < body->stmt_count; i++)
        cco_stmt_free(body->stmts[i]);
    free(body->stmts);
    cco_expr_free(body->final_expr);
    free(body);
}
