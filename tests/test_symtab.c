#include <cnt/cco.h>
#include <stdio.h>
#include <string.h>

static int tests_passed = 0;
static int tests_failed = 0;

#define TEST(name) do { printf("  TEST: %s ... ", name); } while(0)
#define PASS() do { printf("PASS\n"); tests_passed++; } while(0)
#define FAIL(msg) do { printf("FAIL: %s\n", msg); tests_failed++; } while(0)

static void test_symtab_basic(void) {
    cco_symbol_table_t *st = cco_symbol_table_new();

    TEST("symtab create");
    if (st) PASS(); else FAIL("create failed");

    TEST("add typedef");
    cco_type_expr_t *t = cco_type_primitive(CCO_PRIM_STRING);
    int r = cco_symbol_table_add_typedef(st, "Url", t);
    if (r == 0) PASS(); else FAIL("add typedef failed");

    TEST("lookup typedef");
    cco_type_expr_t *found = cco_symbol_table_lookup_type(st, "Url");
    if (found) PASS(); else FAIL("lookup failed");

    TEST("lookup nonexistent");
    found = cco_symbol_table_lookup_type(st, "Nope");
    if (!found) PASS(); else FAIL("should be NULL");

    TEST("duplicate name rejected");
    t = cco_type_primitive(CCO_PRIM_INTEGER);
    r = cco_symbol_table_add_typedef(st, "Url", t);
    if (r != 0) PASS(); else FAIL("should reject duplicate");

    cco_symbol_table_free(st);
}

static void test_symtab_enum(void) {
    cco_symbol_table_t *st = cco_symbol_table_new();
    const char *vals[] = {"GET", "POST", "PUT", NULL};

    TEST("add enum");
    int r = cco_symbol_table_add_enum(st, "HttpMethod", vals);
    if (r == 0) PASS(); else FAIL("add enum failed");

    TEST("has enum value");
    if (cco_symbol_table_has_enum_value(st, "HttpMethod", "GET")) PASS();
    else FAIL("has GET failed");

    TEST("missing enum value");
    if (!cco_symbol_table_has_enum_value(st, "HttpMethod", "DELETE")) PASS();
    else FAIL("should not have DELETE");

    cco_symbol_table_free(st);
}

static void test_parse_full(void) {
    const char *src =
        "$typedef.Url: String,\n"
        "$enum.HttpMethod = (GET, POST),\n"
        "api: \"https://example.com\"\n";

    TEST("parse full with declarations");
    cco_parse_result_t *res = cco_parse_full(src);
    if (res) {
        cco_symbol_table_t *st = cco_parse_result_symbols(res);
        cco_object_t *root = cco_parse_result_root(res);
        if (st && root) {
            cco_type_expr_t *t = cco_symbol_table_lookup_type(st, "Url");
            int has_get = cco_symbol_table_has_enum_value(st, "HttpMethod", "GET");
            cco_object_t *api = cco_object_get(root, "api");
            if (t && has_get && api && cco_is_string(api)) PASS();
            else FAIL("symbols or root incorrect");
        } else FAIL("null symbols or root");
        cco_parse_result_free(res);
    } else {
        FAIL("parse_full returned NULL");
        printf("  error: %s\n", cco_last_error());
    }
}

int main(void) {
    printf("=== libcco symbol table tests ===\n");
    test_symtab_basic();
    test_symtab_enum();
    test_parse_full();
    printf("\nResults: %d passed, %d failed\n", tests_passed, tests_failed);
    return tests_failed > 0 ? 1 : 0;
}
