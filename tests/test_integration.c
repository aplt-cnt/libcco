#include <cnt/cco.h>
#include <stdio.h>
#include <string.h>

static int tests_passed = 0;
static int tests_failed = 0;

#define TEST(name) do { printf("  TEST: %s ... ", name); } while(0)
#define PASS() do { printf("PASS\n"); tests_passed++; } while(0)
#define FAIL(msg) do { printf("FAIL: %s\n", msg); tests_failed++; } while(0)

static void test_subtype(void) {
    /* Test subtype via instantiation with inheritance chain */
    const char *src =
        "$temp.Shape: ( area: Float ),\n"
        "$temp.Circle + Shape: ( radius: Float ),\n"
        "$temp.SmallCircle + Circle: ( label: String ),\n"
        "c: #Circle(3.14, 1.5)\n";

    cco_parse_result_t *res = cco_parse_full(src);
    if (!res) { FAIL(cco_last_error()); return; }

    cco_object_t *c = cco_object_get(cco_parse_result_root(res), "c");
    if (!c || !cco_is_template_instance(c)) { FAIL("no instance"); cco_parse_result_free(res); return; }

    const char *tname = cco_object_template_name(c);
    /* The subtype chain is verified implicitly: Circle fields include Shape's area */
    cco_object_t *area = cco_object_get(c, "area");
    cco_object_t *radius = cco_object_get(c, "radius");

    if (area && radius && cco_is_float(area) && cco_is_float(radius) &&
        cco_float_get(area) == 3.14 && cco_float_get(radius) == 1.5) PASS();
    else FAIL("inherited fields wrong");

    cco_parse_result_free(res);
}

static void test_full_serialize_basic(void) {
    const char *src =
        "$typedef.Url: String,\n"
        "$enum.HttpMethod = (GET, POST),\n"
        "api: \"test\"\n";

    cco_parse_result_t *res = cco_parse_full(src);
    if (!res) { FAIL(cco_last_error()); return; }

    char *s = cco_serialize_full_pretty(res, 2);
    if (!s) { FAIL(cco_last_error()); cco_parse_result_free(res); return; }

    /* Re-parse the serialized output */
    cco_parse_result_t *res2 = cco_parse_full(s);
    if (!res2) { FAIL("re-parse failed"); free(s); cco_parse_result_free(res); return; }

    cco_object_t *root2 = cco_parse_result_root(res2);
    cco_object_t *api = cco_object_get(root2, "api");
    if (api && cco_is_string(api) && strcmp(cco_string_get(api), "test") == 0) {
        /* Check symbols are preserved */
        cco_symbol_table_t *st2 = cco_parse_result_symbols(res2);
        cco_type_expr_t *url_t = NULL;
        if (st2) url_t = cco_symbol_table_lookup_type(st2, "Url");
        if (url_t) PASS();
        else FAIL("typedef not preserved");
    } else FAIL("root value wrong");

    free(s);
    cco_parse_result_free(res);
    cco_parse_result_free(res2);
}

static void test_full_serialize_template(void) {
    const char *src =
        "$temp.Point: ( x: Integer, y: Integer ),\n"
        "origin: #Point(0, 0)\n";

    cco_parse_result_t *res = cco_parse_full(src);
    if (!res) { FAIL(cco_last_error()); return; }

    char *s = cco_serialize_full(res);
    if (!s) { FAIL(cco_last_error()); cco_parse_result_free(res); return; }

    cco_parse_result_t *res2 = cco_parse_full(s);
    if (!res2) { FAIL("re-parse failed"); free(s); cco_parse_result_free(res); return; }

    cco_object_t *root2 = cco_parse_result_root(res2);
    cco_object_t *origin = cco_object_get(root2, "origin");
    if (origin) {
        cco_object_t *x = cco_object_get(origin, "x");
        cco_object_t *y = cco_object_get(origin, "y");
        if (x && y && cco_int_get(x) == 0 && cco_int_get(y) == 0) PASS();
        else FAIL("field values wrong");
    } else FAIL("origin not found");

    free(s);
    cco_parse_result_free(res);
    cco_parse_result_free(res2);
}

int main(void) {
    printf("=== libcco P6+P7 integration tests ===\n");
    test_subtype();
    test_full_serialize_basic();
    test_full_serialize_template();
    printf("\nResults: %d passed, %d failed\n", tests_passed, tests_failed);
    return tests_failed > 0 ? 1 : 0;
}
