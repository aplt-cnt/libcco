#include <cnt/cco.h>
#include <stdio.h>
#include <string.h>

static int tests_passed = 0;
static int tests_failed = 0;

#define TEST(name) do { printf("  TEST: %s ... ", name); } while(0)
#define PASS() do { printf("PASS\n"); tests_passed++; } while(0)
#define FAIL(msg) do { printf("FAIL: %s\n", msg); tests_failed++; } while(0)

static void test_primitive_types(void) {
    cco_type_expr_t *t;

    TEST("type_primitive(String)");
    t = cco_type_primitive(CCO_PRIM_STRING);
    if (t) PASS(); else FAIL("create failed");
    cco_type_expr_free(t);

    TEST("type_primitive(Integer)");
    t = cco_type_primitive(CCO_PRIM_INTEGER);
    if (t) PASS(); else FAIL("create failed");
    cco_type_expr_free(t);

    TEST("type_primitive(Boolean)");
    t = cco_type_primitive(CCO_PRIM_BOOLEAN);
    if (t) PASS(); else FAIL("create failed");
    cco_type_expr_free(t);
}

static void test_array_type(void) {
    cco_type_expr_t *elem = cco_type_primitive(CCO_PRIM_INTEGER);
    cco_type_expr_t *arr  = cco_type_array(elem);

    TEST("type_array");
    if (arr) PASS(); else FAIL("create failed");
    cco_type_expr_free(arr);
}

static void test_object_type(void) {
    cco_field_type_pair_t fields[2];
    fields[0].name = "x";
    fields[0].type = cco_type_primitive(CCO_PRIM_INTEGER);
    fields[1].name = "y";
    fields[1].type = cco_type_primitive(CCO_PRIM_INTEGER);

    cco_type_expr_t *obj = cco_type_object(fields, 2);

    TEST("type_object");
    if (obj) PASS(); else FAIL("create failed");
    cco_type_expr_free(obj);
}

static void test_named_type(void) {
    cco_type_expr_t *t = cco_type_named("HttpMethod");
    TEST("type_named");
    if (t) PASS(); else FAIL("create failed");
    cco_type_expr_free(t);
}

static void test_dyn_type(void) {
    cco_type_expr_t *t = cco_type_dyn("Shape");
    TEST("type_dyn");
    if (t) PASS(); else FAIL("create failed");
    cco_type_expr_free(t);
}

static void test_equal(void) {
    cco_type_expr_t *a = cco_type_primitive(CCO_PRIM_INTEGER);
    cco_type_expr_t *b = cco_type_primitive(CCO_PRIM_INTEGER);
    cco_type_expr_t *c = cco_type_primitive(CCO_PRIM_STRING);

    TEST("type_equal same");
    if (cco_type_expr_equal(a, b)) PASS(); else FAIL("should be equal");
    TEST("type_equal different");
    if (!cco_type_expr_equal(a, c)) PASS(); else FAIL("should differ");

    cco_type_expr_free(a);
    cco_type_expr_free(b);
    cco_type_expr_free(c);
}

static void test_copy(void) {
    cco_type_expr_t *orig = cco_type_primitive(CCO_PRIM_FLOAT);
    cco_type_expr_t *copy = cco_type_expr_copy(orig);

    TEST("type_copy");
    if (copy && cco_type_expr_equal(orig, copy)) PASS();
    else FAIL("copy mismatch");

    cco_type_expr_free(orig);
    cco_type_expr_free(copy);
}

int main(void) {
    printf("=== libcco type system tests ===\n");
    test_primitive_types();
    test_array_type();
    test_object_type();
    test_named_type();
    test_dyn_type();
    test_equal();
    test_copy();
    printf("\nResults: %d passed, %d failed\n", tests_passed, tests_failed);
    return tests_failed > 0 ? 1 : 0;
}
