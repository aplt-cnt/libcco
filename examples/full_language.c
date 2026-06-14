/**
 * full_language.c — Demonstrates the full CCO language:
 *   $typedef, $enum, $temp, inheritance, $format, expressions, serialization.
 *
 * Compile: cc -I../include full_language.c -L../target/Debug -lcco
 */
#include <cnt/cco.h>
#include <stdio.h>

int main(void) {
    /* ---- 1. Parse a full CCO file ---- */
    const char *src =
        "$typedef.Url: String,\n"
        "$enum.HttpMethod = (GET, POST, PUT, DELETE),\n"
        "$temp.Endpoint: (\n"
        "    url: Url,\n"
        "    method: HttpMethod\n"
        "),\n"
        "$temp.SecureEndpoint + Endpoint: (\n"
        "    port: Integer = 443\n"
        "),\n"
        "endpoints: (\n"
        "    api:   #Endpoint(\"https://api.example.com\", GET),\n"
        "    admin: #SecureEndpoint(\"https://admin.example.com\", POST, 8443)\n"
        ")\n";

    cco_parse_result_t *res = cco_parse_full(src);
    if (!res) {
        fprintf(stderr, "Parse error: %s\n", cco_last_error());
        return 1;
    }

    /* ---- 2. Inspect the symbol table ---- */
    cco_symbol_table_t *st = cco_parse_result_symbols(res);
    printf("Defined types: Url, HttpMethod, Endpoint, SecureEndpoint\n\n");

    /* ---- 3. Read values ---- */
    cco_object_t *eps = cco_object_get(cco_parse_result_root(res), "endpoints");

    cco_object_t *api = cco_object_get(eps, "api");
    printf("api.url    = %s\n", cco_string_get(cco_object_get(api, "url")));
    printf("api.method = %lld (GET=0, POST=1, ...)\n",
           cco_int_get(cco_object_get(api, "method")));

    cco_object_t *admin = cco_object_get(eps, "admin");
    printf("admin.url    = %s\n", cco_string_get(cco_object_get(admin, "url")));
    printf("admin.port   = %lld\n", cco_int_get(cco_object_get(admin, "port")));
    printf("admin.method = %lld\n", cco_int_get(cco_object_get(admin, "method")));

    /* ---- 4. Expressions ($(...) with operators) ---- */
    printf("\nExpressions:\n");
    cco_expr_t *ex = cco_expr_binary(cco_expr_literal(cco_int_new(10)),
                                      CCO_OP_ADD,
                                      cco_expr_binary(cco_expr_literal(cco_int_new(20)),
                                                       CCO_OP_MUL,
                                                       cco_expr_literal(cco_int_new(3))));
    cco_object_t *ev = cco_expr_eval(ex, st, NULL, NULL);
    printf("  10 + 20 * 3 = %lld\n", cco_int_get(ev));
    cco_object_free(ev);
    cco_expr_free(ex);

    /* ---- 5. $format ---- */
    printf("\n$format:\n");
    cco_expr_t *fmt = cco_expr_format(cco_expr_literal(cco_string_new("(1, 2, 3)")));
    cco_object_t *arr = cco_expr_eval(fmt, st, NULL, NULL);
    const cco_array_t *a = cco_object_as_array(arr);
    for (size_t i = 0; i < cco_array_size(a); i++)
        printf("  [%zu] = %lld\n", i, cco_int_get(cco_array_get(a, i)));
    cco_object_free(arr);
    cco_expr_free(fmt);

    /* ---- 6. Full serialization roundtrip ---- */
    char *out = cco_serialize_full(res);
    printf("\nSerialized:\n%s\n", out);
    free(out);

    cco_parse_result_free(res);
    return 0;
}
