#include <cnt/cco.h>
#include <stdio.h>
#include <string.h>

static int tests_passed = 0;
static int tests_failed = 0;

#define TEST(name) do { printf("  TEST: %s ... ", name); } while(0)
#define PASS() do { printf("PASS\n"); tests_passed++; } while(0)
#define FAIL(msg) do { printf("FAIL: %s\n", msg); tests_failed++; } while(0)

static void test_basic_instantiate(void) {
    TEST("basic $temp + # instantiation");
    cco_parse_result_t *res = cco_parse_full(
        "$temp.Point: ( x: Integer, y: Integer ),\n"
        "origin: #Point(0, 0)\n");
    if (!res) { FAIL(cco_last_error()); return; }
    cco_object_t *origin = cco_object_get(cco_parse_result_root(res), "origin");
    if (origin && cco_is_template_instance(origin) &&
        strcmp(cco_object_template_name(origin), "Point") == 0) {
        cco_object_t *x = cco_object_get(origin, "x");
        cco_object_t *y = cco_object_get(origin, "y");
        if (x && y && cco_int_get(x) == 0 && cco_int_get(y) == 0) PASS();
        else FAIL("field values wrong");
    } else FAIL("bad instance");
    cco_parse_result_free(res);
}

static void test_default_value(void) {
    TEST("template with default value");
    cco_parse_result_t *res = cco_parse_full(
        "$temp.Endpt: ( url: String, port: Integer = 443 ),\n"
        "api: #Endpt(\"https://ex.com\")\n");
    if (!res) { FAIL(cco_last_error()); return; }
    cco_object_t *api = cco_object_get(cco_parse_result_root(res), "api");
    if (api) {
        cco_object_t *port = cco_object_get(api, "port");
        if (port && cco_int_get(port) == 443) PASS();
        else FAIL("default wrong");
    } else FAIL("no instance");
    cco_parse_result_free(res);
}

static void test_inheritance(void) {
    TEST("inheritance + field merging");
    cco_parse_result_t *res = cco_parse_full(
        "$temp.Base: ( id: Integer ),\n"
        "$temp.Derived + Base: ( name: String ),\n"
        "obj: #Derived(1, \"test\")\n");
    if (!res) { FAIL(cco_last_error()); return; }
    cco_object_t *obj = cco_object_get(cco_parse_result_root(res), "obj");
    if (obj && cco_is_template_instance(obj)) {
        cco_object_t *id = cco_object_get(obj, "id");
        cco_object_t *name = cco_object_get(obj, "name");
        if (id && name && cco_int_get(id) == 1 &&
            strcmp(cco_string_get(name), "test") == 0) PASS();
        else FAIL("inherited field wrong");
    } else FAIL("not instance");
    cco_parse_result_free(res);
}

static void test_named_instantiation(void) {
    TEST("named (field-by-field) instantiation");
    cco_parse_result_t *res = cco_parse_full(
        "$temp.Point: ( x: Integer, y: Integer ),\n"
        "p: #Point(.x: 10, .y: 20)\n");
    if (!res) { FAIL(cco_last_error()); return; }
    cco_object_t *p = cco_object_get(cco_parse_result_root(res), "p");
    if (p && cco_is_template_instance(p)) {
        cco_object_t *x = cco_object_get(p, "x");
        cco_object_t *y = cco_object_get(p, "y");
        if (x && y && cco_int_get(x) == 10 && cco_int_get(y) == 20) PASS();
        else FAIL("named field wrong");
    } else FAIL("not instance");
    cco_parse_result_free(res);
}

static void test_inherited_named(void) {
    TEST("inherited + named instantiation");
    cco_parse_result_t *res = cco_parse_full(
        "$temp.Base: ( id: Integer ),\n"
        "$temp.Derived + Base: ( name: String ),\n"
        "obj: #Derived(.id: 5, .name: \"x\")\n");
    if (!res) { FAIL(cco_last_error()); return; }
    cco_object_t *obj = cco_object_get(cco_parse_result_root(res), "obj");
    if (obj && cco_is_template_instance(obj)) {
        cco_object_t *id = cco_object_get(obj, "id");
        cco_object_t *name = cco_object_get(obj, "name");
        if (id && name && cco_int_get(id) == 5 &&
            strcmp(cco_string_get(name), "x") == 0) PASS();
        else FAIL("inherited named field wrong");
    } else FAIL("not instance");
    cco_parse_result_free(res);
}

int main(void) {
    printf("=== libcco Phase 4 tests ===\n");
    test_basic_instantiate();
    test_default_value();
    test_inheritance();
    test_named_instantiation();
    test_inherited_named();
    printf("\nResults: %d passed, %d failed\n", tests_passed, tests_failed);
    return tests_failed > 0 ? 1 : 0;
}
