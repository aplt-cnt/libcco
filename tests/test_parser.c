#include <cnt/cco.h>
#include <stdio.h>
#include <string.h>

static int tests_passed = 0;
static int tests_failed = 0;

#define TEST(name) do { printf("  TEST: %s ... ", name); } while(0)
#define PASS() do { printf("PASS\n"); tests_passed++; } while(0)
#define FAIL(msg) do { printf("FAIL: %s\n", msg); tests_failed++; } while(0)

static void test_parse_primitives(void) {
    cco_object_t *v;

    TEST("parse None");
    v = cco_parse("None");
    if (v && cco_is_none(v)) PASS(); else FAIL("None parse failed");
    cco_object_free(v);

    TEST("parse true");
    v = cco_parse("true");
    if (v && cco_is_bool(v) && cco_bool_get(v) == 1) PASS(); else FAIL("true parse failed");
    cco_object_free(v);

    TEST("parse false");
    v = cco_parse("false");
    if (v && cco_is_bool(v) && cco_bool_get(v) == 0) PASS(); else FAIL("false parse failed");
    cco_object_free(v);

    TEST("parse integer");
    v = cco_parse("42");
    if (v && cco_is_int(v) && cco_int_get(v) == 42) PASS(); else FAIL("int parse failed");
    cco_object_free(v);

    TEST("parse float");
    v = cco_parse("3.14");
    if (v && cco_is_float(v)) PASS(); else FAIL("float parse failed");
    cco_object_free(v);

    TEST("parse string");
    v = cco_parse("\"hello\"");
    if (v && cco_is_string(v) && strcmp(cco_string_get(v), "hello") == 0) PASS();
    else FAIL("string parse failed");
    cco_object_free(v);

    TEST("parse hex int");
    v = cco_parse("0xFF");
    if (v && cco_is_int(v) && cco_int_get(v) == 255) PASS();
    else FAIL("hex parse failed");
    cco_object_free(v);

    TEST("parse octal int");
    v = cco_parse("0o10");
    if (v && cco_is_int(v) && cco_int_get(v) == 8) PASS();
    else FAIL("octal parse failed");
    cco_object_free(v);

    TEST("parse underscore in int");
    v = cco_parse("1_000_000");
    if (v && cco_is_int(v) && cco_int_get(v) == 1000000) PASS();
    else FAIL("underscore int failed");
    cco_object_free(v);
}

static void test_parse_map(void) {
    cco_object_t *v;

    TEST("parse empty map");
    v = cco_parse("()");
    if (v && cco_is_map(v) && cco_object_size(v) == 0) PASS();
    else FAIL("empty map failed");
    cco_object_free(v);

    TEST("parse map one field");
    v = cco_parse("(x: 1)");
    if (v && cco_is_map(v) && cco_object_size(v) == 1) {
        cco_object_t *fv = cco_object_get(v, "x");
        if (fv && cco_is_int(fv) && cco_int_get(fv) == 1) PASS();
        else FAIL("map field value wrong");
    } else FAIL("map one field failed");
    cco_object_free(v);

    TEST("parse map multi field");
    v = cco_parse("(a: 1, b: \"two\")");
    if (v && cco_is_map(v) && cco_object_size(v) == 2) {
        cco_object_t *fa = cco_object_get(v, "a");
        cco_object_t *fb = cco_object_get(v, "b");
        if (fa && fb && cco_is_int(fa) && cco_is_string(fb)) PASS();
        else FAIL("multi field values wrong");
    } else FAIL("multi field failed");
    cco_object_free(v);

    TEST("parse top-level shorthand");
    v = cco_parse("name: \"test\"");
    if (v && cco_is_map(v) && cco_object_size(v) == 1) {
        cco_object_t *fv = cco_object_get(v, "name");
        if (fv && cco_is_string(fv) && strcmp(cco_string_get(fv), "test") == 0) PASS();
        else FAIL("shorthand value wrong");
    } else FAIL("shorthand parse failed");
    cco_object_free(v);
}

static void test_parse_array(void) {
    cco_object_t *v;

    TEST("parse array one element");
    v = cco_parse("(42)");
    if (v && cco_is_array(v)) {
        const cco_array_t *a = cco_object_as_array(v);
        if (cco_array_size(a) == 1) {
            cco_object_t *e = cco_array_get(a, 0);
            if (e && cco_is_int(e) && cco_int_get(e) == 42) PASS();
            else FAIL("array element wrong");
        } else FAIL("array size wrong");
    } else FAIL("array one element failed");
    cco_object_free(v);

    TEST("parse array multi");
    v = cco_parse("(1, 2, 3)");
    if (v && cco_is_array(v)) {
        const cco_array_t *a = cco_object_as_array(v);
        if (cco_array_size(a) == 3) PASS();
        else FAIL("array multi count wrong");
    } else FAIL("array multi failed");
    cco_object_free(v);
}

static void test_roundtrip(void) {
    const char *inputs[] = {
        "None",
        "true",
        "false",
        "42",
        "3.14",
        "\"hello\"",
        "\"esc\\\"aped\"",
        "(x: 1)",
        "(a: 1, b: \"two\")",
        "(1, 2, 3)",
        NULL
    };

    for (int i = 0; inputs[i]; i++) {
        char label[64];
        snprintf(label, sizeof(label), "roundtrip %d: %s", i, inputs[i]);
        TEST(label);
        cco_object_t *v = cco_parse(inputs[i]);
        if (!v) { FAIL("parse failed"); continue; }
        char *s = cco_serialize(v);
        if (!s) { FAIL("serialize failed"); cco_object_free(v); continue; }
        cco_object_t *v2 = cco_parse(s);
        if (!v2) { FAIL("re-parse failed"); free(s); cco_object_free(v); continue; }
        char *s2 = cco_serialize(v2);
        if (!s2) { FAIL("re-serialize failed"); free(s); cco_object_free(v);
                   cco_object_free(v2); continue; }
        if (strcmp(s, s2) == 0) PASS();
        else { FAIL("mismatch"); printf("      s1=%s\n      s2=%s\n", s, s2); }
        free(s); free(s2);
        cco_object_free(v);
        cco_object_free(v2);
    }
}

int main(void) {
    printf("=== libcco parser tests ===\n");
    test_parse_primitives();
    test_parse_map();
    test_parse_array();
    test_roundtrip();
    printf("\nResults: %d passed, %d failed\n", tests_passed, tests_failed);
    return tests_failed > 0 ? 1 : 0;
}
