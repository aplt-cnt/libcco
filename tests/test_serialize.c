#include <cnt/cco.h>
#include <stdio.h>
#include <string.h>

static int tests_passed = 0;
static int tests_failed = 0;

#define TEST(name) do { printf("  TEST: %s ... ", name); } while(0)
#define PASS() do { printf("PASS\n"); tests_passed++; } while(0)
#define FAIL(msg) do { printf("FAIL: %s\n", msg); tests_failed++; } while(0)

static void test_serialize_primitives(void) {
    char *s;

    TEST("serialize None");
    s = cco_serialize(cco_none_new());
    if (s && strcmp(s, "None") == 0) PASS(); else FAIL(s);
    free(s);
    cco_clear_error();

    TEST("serialize true");
    s = cco_serialize(cco_bool_new(1));
    if (s && strcmp(s, "true") == 0) PASS(); else FAIL(s);
    free(s);

    TEST("serialize false");
    s = cco_serialize(cco_bool_new(0));
    if (s && strcmp(s, "false") == 0) PASS(); else FAIL(s);
    free(s);

    TEST("serialize int");
    s = cco_serialize(cco_int_new(42));
    if (s && strcmp(s, "42") == 0) PASS(); else FAIL(s);
    free(s);

    TEST("serialize negative int");
    s = cco_serialize(cco_int_new(-100));
    if (s && strcmp(s, "-100") == 0) PASS(); else FAIL(s);
    free(s);

    TEST("serialize float");
    s = cco_serialize(cco_float_new(3.5));
    if (s && strcmp(s, "3.5") == 0) PASS(); else FAIL(s);
    free(s);

    TEST("serialize string");
    s = cco_serialize(cco_string_new("hello"));
    if (s && strcmp(s, "\"hello\"") == 0) PASS(); else FAIL(s);
    free(s);

    TEST("serialize string with escape");
    s = cco_serialize(cco_string_new("a\"b"));
    if (s && strcmp(s, "\"a\\\"b\"") == 0) PASS(); else FAIL(s);
    free(s);
}

static void test_serialize_map(void) {
    cco_object_t *m = cco_object_new();
    cco_object_set(m, "name", cco_string_new("test"));
    cco_object_set(m, "count", cco_int_new(3));

    TEST("serialize map compact");
    char *s = cco_serialize(m);
    /* Compact single-line format */
    if (s) PASS(); else FAIL("serialize map returned NULL");
    printf("    got: %s\n", s);
    free(s);
    cco_object_free(m);
}

static void test_serialize_array(void) {
    cco_array_t *a = cco_array_new();
    cco_array_add(a, cco_int_new(1));
    cco_array_add(a, cco_int_new(2));
    cco_object_t *o = cco_array_wrap(a);

    TEST("serialize array");
    char *s = cco_serialize(o);
    if (s) PASS(); else FAIL("serialize array returned NULL");
    printf("    got: %s\n", s);
    free(s);
    cco_object_free(o);
}

int main(void) {
    printf("=== libcco serialization tests ===\n");
    test_serialize_primitives();
    test_serialize_map();
    test_serialize_array();
    printf("\nResults: %d passed, %d failed\n", tests_passed, tests_failed);
    return tests_failed > 0 ? 1 : 0;
}
