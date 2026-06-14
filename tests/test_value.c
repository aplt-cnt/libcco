#include <cnt/cco.h>
#include <stdio.h>
#include <string.h>

static int tests_passed = 0;
static int tests_failed = 0;

#define TEST(name) do { printf("  TEST: %s ... ", name); } while(0)
#define PASS() do { printf("PASS\n"); tests_passed++; } while(0)
#define FAIL(msg) do { printf("FAIL: %s\n", msg); tests_failed++; } while(0)

static void test_primitive_types(void) {
    TEST("none creation");
    cco_object_t *n = cco_none_new();
    if (n && cco_is_none(n)) PASS(); else FAIL("none failed");
    cco_object_free(n);

    TEST("bool true");
    cco_object_t *bt = cco_bool_new(1);
    if (bt && cco_is_bool(bt) && cco_bool_get(bt) == 1) PASS();
    else FAIL("bool true failed");
    cco_object_free(bt);

    TEST("bool false");
    cco_object_t *bf = cco_bool_new(0);
    if (bf && cco_is_bool(bf) && cco_bool_get(bf) == 0) PASS();
    else FAIL("bool false failed");
    cco_object_free(bf);

    TEST("integer");
    cco_object_t *i = cco_int_new(42);
    if (i && cco_is_int(i) && cco_int_get(i) == 42) PASS();
    else FAIL("int failed");
    cco_object_free(i);

    TEST("negative integer");
    cco_object_t *ni = cco_int_new(-100);
    if (ni && cco_int_get(ni) == -100) PASS();
    else FAIL("negative int failed");
    cco_object_free(ni);

    TEST("float");
    cco_object_t *f = cco_float_new(3.14);
    if (f && cco_is_float(f) && cco_float_get(f) > 3.13 && cco_float_get(f) < 3.15) PASS();
    else FAIL("float failed");
    cco_object_free(f);

    TEST("string");
    cco_object_t *s = cco_string_new("hello");
    if (s && cco_is_string(s) && strcmp(cco_string_get(s), "hello") == 0) PASS();
    else FAIL("string failed");
    cco_object_free(s);
}

static void test_map_operations(void) {
    TEST("map create empty");
    cco_object_t *m = cco_object_new();
    if (m && cco_is_map(m) && cco_object_size(m) == 0) PASS();
    else FAIL("map create failed");
    cco_object_free(m);

    TEST("map set and get");
    m = cco_object_new();
    cco_object_set(m, "name", cco_string_new("Alice"));
    cco_object_t *v = cco_object_get(m, "name");
    if (v && cco_is_string(v) && strcmp(cco_string_get(v), "Alice") == 0) PASS();
    else FAIL("set/get failed");
    cco_object_free(m);

    TEST("map overwrite");
    m = cco_object_new();
    cco_object_set(m, "x", cco_int_new(1));
    cco_object_set(m, "x", cco_int_new(2));
    v = cco_object_get(m, "x");
    if (v && cco_int_get(v) == 2) PASS();
    else FAIL("overwrite failed");
    cco_object_free(m);

    TEST("map has_key");
    m = cco_object_new();
    cco_object_set(m, "a", cco_int_new(1));
    if (cco_object_has_key(m, "a") && !cco_object_has_key(m, "b")) PASS();
    else FAIL("has_key failed");
    cco_object_free(m);

    TEST("map delete");
    m = cco_object_new();
    cco_object_set(m, "a", cco_int_new(1));
    cco_object_set(m, "b", cco_int_new(2));
    cco_object_del(m, "a");
    if (!cco_object_has_key(m, "a") && cco_object_has_key(m, "b") &&
        cco_object_size(m) == 1) PASS();
    else FAIL("delete failed");
    cco_object_free(m);

    TEST("map clear");
    m = cco_object_new();
    cco_object_set(m, "a", cco_int_new(1));
    cco_object_clear(m);
    if (cco_object_size(m) == 0) PASS();
    else FAIL("clear failed");
    cco_object_free(m);
}

static void test_array_operations(void) {
    TEST("array create empty");
    cco_array_t *a = cco_array_new();
    if (a && cco_array_size(a) == 0) PASS();
    else FAIL("array create failed");
    cco_array_free(a);

    TEST("array add and get");
    a = cco_array_new();
    cco_array_add(a, cco_int_new(10));
    cco_array_add(a, cco_int_new(20));
    cco_object_t *v = cco_array_get(a, 1);
    if (cco_array_size(a) == 2 && v && cco_int_get(v) == 20) PASS();
    else FAIL("add/get failed");
    cco_array_free(a);

    TEST("array insert");
    a = cco_array_new();
    cco_array_add(a, cco_int_new(1));
    cco_array_add(a, cco_int_new(3));
    cco_array_insert(a, 1, cco_int_new(2));
    v = cco_array_get(a, 1);
    if (cco_array_size(a) == 3 && v && cco_int_get(v) == 2) PASS();
    else FAIL("insert failed");
    cco_array_free(a);

    TEST("array remove");
    a = cco_array_new();
    cco_array_add(a, cco_int_new(1));
    cco_array_add(a, cco_int_new(2));
    cco_array_add(a, cco_int_new(3));
    cco_array_remove(a, 1);
    v = cco_array_get(a, 1);
    if (cco_array_size(a) == 2 && v && cco_int_get(v) == 3) PASS();
    else FAIL("remove failed");
    cco_array_free(a);

    TEST("array wrap");
    a = cco_array_new();
    cco_array_add(a, cco_string_new("x"));
    cco_object_t *o = cco_array_wrap(a);
    if (o && cco_is_array(o)) PASS();
    else FAIL("wrap failed");
    cco_object_free(o);
}

static void test_copy(void) {
    TEST("deep copy int");
    cco_object_t *i = cco_int_new(42);
    cco_object_t *c = cco_object_copy(i);
    if (c && cco_is_int(c) && cco_int_get(c) == 42) PASS();
    else FAIL("copy int failed");
    cco_object_free(i);
    cco_object_free(c);

    TEST("deep copy map");
    cco_object_t *m = cco_object_new();
    cco_object_set(m, "a", cco_int_new(1));
    cco_object_set(m, "b", cco_string_new("hi"));
    cco_object_t *mc = cco_object_copy(m);
    int ok = mc && cco_is_map(mc) && cco_object_size(mc) == 2;
    cco_object_t *va = cco_object_get(mc, "a");
    if (ok && va && cco_int_get(va) == 1) PASS();
    else FAIL("copy map failed");
    cco_object_free(m);
    cco_object_free(mc);
}

static void test_template_instance(void) {
    TEST("template instance creation");
    cco_object_t *ti = cco_template_instance_new("Point");
    if (ti && cco_is_template_instance(ti) &&
        strcmp(cco_object_template_name(ti), "Point") == 0) PASS();
    else FAIL("template instance creation failed");
    cco_object_free(ti);

    TEST("template instance field access");
    ti = cco_template_instance_new("User");
    cco_object_set(ti, "name", cco_string_new("Alice"));
    cco_object_t *v = cco_object_get(ti, "name");
    if (v && cco_is_string(v) && strcmp(cco_string_get(v), "Alice") == 0) PASS();
    else FAIL("template instance field failed");
    cco_object_free(ti);
}

static void test_iterator(void) {
    TEST("map iterator");
    cco_object_t *m = cco_object_new();
    cco_object_set(m, "a", cco_int_new(1));
    cco_object_set(m, "b", cco_int_new(2));
    cco_map_iter_t *it = cco_map_iter_new(m);
    const char *k;
    cco_object_t *v;
    int count = 0;
    while (cco_map_iter_next(it, &k, &v)) count++;
    cco_map_iter_free(it);
    if (count == 2) PASS();
    else FAIL("iterator count wrong");
    cco_object_free(m);
}

int main(void) {
    printf("=== libcco value layer tests ===\n");

    test_primitive_types();
    test_map_operations();
    test_array_operations();
    test_copy();
    test_template_instance();
    test_iterator();

    printf("\nResults: %d passed, %d failed\n", tests_passed, tests_failed);
    return tests_failed > 0 ? 1 : 0;
}
