/**
 * cco_file_demo.c — Read, modify, and write .cco files.
 *
 * Compile: cc -I../include cco_file_demo.c -L../target/Debug -lcco
 */
#include <cnt/cco.h>
#include <stdio.h>
#include <string.h>

int main(void) {
    /* Write a .cco file */
    {
        cco_object_t *cfg = cco_object_new();
        cco_object_set(cfg, "host",    cco_string_new("localhost"));
        cco_object_set(cfg, "port",    cco_int_new(8080));
        cco_object_set(cfg, "debug",   cco_bool_new(1));

        cco_array_t *addrs = cco_array_new();
        cco_array_add(addrs, cco_string_new("10.0.0.1"));
        cco_array_add(addrs, cco_string_new("10.0.0.2"));
        cco_object_set(cfg, "peers", cco_array_wrap(addrs));

        cco_save_file_pretty(cfg, "demo_output.cco", 2);
        printf("Written demo_output.cco\n");
        cco_object_free(cfg);
    }

    /* Read it back */
    {
        cco_object_t *cfg = cco_parse_from_file("demo_output.cco");
        if (!cfg) {
            fprintf(stderr, "Read error: %s\n", cco_last_error());
            return 1;
        }

        printf("host  = %s\n", cco_string_get(cco_object_get(cfg, "host")));
        printf("port  = %lld\n", cco_int_get(cco_object_get(cfg, "port")));
        printf("debug = %s\n", cco_bool_get(cco_object_get(cfg, "debug")) ? "true" : "false");

        cco_object_t *peers = cco_object_get(cfg, "peers");
        const cco_array_t *a = cco_object_as_array(peers);
        printf("peers = [");
        for (size_t i = 0; i < cco_array_size(a); i++) {
            if (i > 0) printf(", ");
            printf("\"%s\"", cco_string_get(cco_array_get(a, i)));
        }
        printf("]\n");

        cco_object_free(cfg);
    }

    return 0;
}
