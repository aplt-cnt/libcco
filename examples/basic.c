/**
 * basic.c — Build .cco values programmatically and serialize them.
 *
 * Compile: cc -I../include basic.c -L../target/Debug -lcco
 * Run: ./a.out
 */
#include <cnt/cco.h>
#include <stdio.h>

int main(void) {
    /* Build a simple config: ( name: "CCO", version: 1, active: true ) */
    cco_object_t *cfg = cco_object_new();
    cco_object_set(cfg, "name",    cco_string_new("CCO"));
    cco_object_set(cfg, "version", cco_int_new(1));
    cco_object_set(cfg, "active",  cco_bool_new(1));

    /* Pretty-print */
    char *text = cco_serialize_pretty(cfg, 2);
    printf("Config:\n%s\n", text);
    free(text);

    /* Compact form */
    text = cco_serialize(cfg);
    printf("Compact: %s\n", text);
    free(text);

    cco_object_free(cfg);
    return 0;
}
