#include <cnt/cco.h>
#include <stdio.h>
#include <string.h>

static int tests_passed = 0;
static int tests_failed = 0;

#define TEST(name) do { printf("  TEST: %s ... ", name); } while(0)
#define PASS() do { printf("PASS\n"); tests_passed++; } while(0)
#define FAIL(msg) do { printf("FAIL: %s\n", msg); tests_failed++; } while(0)

/* Helper: create a literal expression */
static cco_expr_t* lit_int(long long v) { return cco_expr_literal(cco_int_new(v)); }
static cco_expr_t* lit_bool(int v) { return cco_expr_literal(cco_bool_new(v)); }
static cco_expr_t* lit_float(double v) { return cco_expr_literal(cco_float_new(v)); }
static cco_expr_t* lit_none(void) { return cco_expr_literal(cco_none_new()); }
static cco_expr_t* lit_str(const char *s) { return cco_expr_literal(cco_string_new(s)); }

static void test_literal(void) {
    TEST("literal int");
    cco_expr_t *e = lit_int(42);
    cco_object_t *r = cco_expr_eval(e, NULL, NULL, NULL);
    if (r && cco_is_int(r) && cco_int_get(r) == 42) PASS(); else FAIL("");
    cco_object_free(r); cco_expr_free(e);
}

static void test_arithmetic(void) {
    cco_expr_t *e;
    cco_object_t *r;

    TEST("add ints");
    e = cco_expr_binary(lit_int(1), CCO_OP_ADD, lit_int(2));
    r = cco_expr_eval(e, NULL, NULL, NULL);
    if (r && cco_is_int(r) && cco_int_get(r) == 3) PASS(); else FAIL("");
    cco_object_free(r); cco_expr_free(e);

    TEST("sub ints");
    e = cco_expr_binary(lit_int(10), CCO_OP_SUB, lit_int(3));
    r = cco_expr_eval(e, NULL, NULL, NULL);
    if (r && cco_is_int(r) && cco_int_get(r) == 7) PASS(); else FAIL("");
    cco_object_free(r); cco_expr_free(e);

    TEST("mul ints");
    e = cco_expr_binary(lit_int(6), CCO_OP_MUL, lit_int(7));
    r = cco_expr_eval(e, NULL, NULL, NULL);
    if (r && cco_is_int(r) && cco_int_get(r) == 42) PASS(); else FAIL("");
    cco_object_free(r); cco_expr_free(e);

    TEST("div ints");
    e = cco_expr_binary(lit_int(10), CCO_OP_DIV, lit_int(3));
    r = cco_expr_eval(e, NULL, NULL, NULL);
    if (r && cco_is_int(r) && cco_int_get(r) == 3) PASS(); else FAIL("");
    cco_object_free(r); cco_expr_free(e);
}

static void test_comparison(void) {
    cco_expr_t *e;
    cco_object_t *r;

    TEST("eq true");
    e = cco_expr_binary(lit_int(1), CCO_OP_EQ, lit_int(1));
    r = cco_expr_eval(e, NULL, NULL, NULL);
    if (r && cco_is_bool(r) && cco_bool_get(r)) PASS(); else FAIL("");
    cco_object_free(r); cco_expr_free(e);

    TEST("eq false");
    e = cco_expr_binary(lit_int(1), CCO_OP_EQ, lit_int(2));
    r = cco_expr_eval(e, NULL, NULL, NULL);
    if (r && cco_is_bool(r) && !cco_bool_get(r)) PASS(); else FAIL("");
    cco_object_free(r); cco_expr_free(e);

    TEST("lt true");
    e = cco_expr_binary(lit_int(1), CCO_OP_LT, lit_int(2));
    r = cco_expr_eval(e, NULL, NULL, NULL);
    if (r && cco_is_bool(r) && cco_bool_get(r)) PASS(); else FAIL("");
    cco_object_free(r); cco_expr_free(e);
}

static void test_logical(void) {
    cco_expr_t *e;
    cco_object_t *r;

    TEST("and true true");
    e = cco_expr_binary(lit_bool(1), CCO_OP_AND, lit_bool(1));
    r = cco_expr_eval(e, NULL, NULL, NULL);
    if (r && cco_is_bool(r) && cco_bool_get(r)) PASS(); else FAIL("");
    cco_object_free(r); cco_expr_free(e);

    TEST("and true false");
    e = cco_expr_binary(lit_bool(1), CCO_OP_AND, lit_bool(0));
    r = cco_expr_eval(e, NULL, NULL, NULL);
    if (r && cco_is_bool(r) && !cco_bool_get(r)) PASS(); else FAIL("");
    cco_object_free(r); cco_expr_free(e);

    TEST("or false true");
    e = cco_expr_binary(lit_bool(0), CCO_OP_OR, lit_bool(1));
    r = cco_expr_eval(e, NULL, NULL, NULL);
    if (r && cco_is_bool(r) && cco_bool_get(r)) PASS(); else FAIL("");
    cco_object_free(r); cco_expr_free(e);

    TEST("not true");
    e = cco_expr_unary(CCO_OP_NOT, lit_bool(1));
    r = cco_expr_eval(e, NULL, NULL, NULL);
    if (r && cco_is_bool(r) && !cco_bool_get(r)) PASS(); else FAIL("");
    cco_object_free(r); cco_expr_free(e);
}

static void test_coalesce(void) {
    cco_expr_t *e;
    cco_object_t *r;

    TEST("coalesce none fallback");
    e = cco_expr_coalesce(lit_none(), lit_int(42));
    r = cco_expr_eval(e, NULL, NULL, NULL);
    if (r && cco_is_int(r) && cco_int_get(r) == 42) PASS(); else FAIL("");
    cco_object_free(r); cco_expr_free(e);

    TEST("coalesce value used");
    e = cco_expr_coalesce(lit_int(10), lit_int(99));
    r = cco_expr_eval(e, NULL, NULL, NULL);
    if (r && cco_is_int(r) && cco_int_get(r) == 10) PASS(); else FAIL("");
    cco_object_free(r); cco_expr_free(e);
}

static void test_compound(void) {
    TEST("compound wrapper");
    cco_expr_t *e = cco_expr_compound(lit_int(7));
    cco_object_t *r = cco_expr_eval(e, NULL, NULL, NULL);
    if (r && cco_is_int(r) && cco_int_get(r) == 7) PASS(); else FAIL("");
    cco_object_free(r); cco_expr_free(e);
}

static void test_format(void) {
    TEST("$format simple");
    cco_expr_t *e = cco_expr_format(lit_str("42"));
    cco_object_t *r = cco_expr_eval(e, NULL, NULL, NULL);
    if (r && cco_is_int(r) && cco_int_get(r) == 42) PASS(); else FAIL(cco_last_error());
    cco_object_free(r); cco_expr_free(e);
}

int main(void) {
    printf("=== libcco expression evaluator tests ===\n");
    test_literal();
    test_arithmetic();
    test_comparison();
    test_logical();
    test_coalesce();
    test_compound();
    test_format();
    printf("\nResults: %d passed, %d failed\n", tests_passed, tests_failed);
    return tests_failed > 0 ? 1 : 0;
}
