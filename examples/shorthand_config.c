#include <cnt/cco.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/*
 * Demonstrates CCO's ability to parse top-level shorthand maps
 * (files without an enclosing braces {})
 */

int main(void) {
    const char* cco_text = 
        "host: \"127.0.0.1\",\n"
        "port: 8080,\n"
        "debug: true,\n"
        "tags: [\"web\", \"backend\"]\n";

    printf("Parsing Shorthand Configuration...\n");
    cco_object_t* config = cco_parse_string(cco_text, strlen(cco_text), NULL);
    if (!config) {
        printf("Parse failed with error %d\n", cco_get_last_error());
        return 1;
    }

    if (cco_object_get_type(config) == CCO_TYPE_MAP) {
        cco_object_t* host_obj = cco_map_get_by_key(config, "host");
        cco_object_t* port_obj = cco_map_get_by_key(config, "port");
        cco_object_t* debug_obj = cco_map_get_by_key(config, "debug");

        const char* host; size_t host_len;
        if (host_obj && cco_object_get_string(host_obj, &host, &host_len)) {
            printf("Host: %s\n", host);
        }

        int64_t port;
        if (port_obj && cco_object_get_integer(port_obj, &port)) {
            printf("Port: %lld\n", (long long)port);
        }

        bool debug;
        if (debug_obj && cco_object_get_boolean(debug_obj, &debug)) {
            printf("Debug: %s\n", debug ? "ON" : "OFF");
        }
    }

    cco_object_release(config);
    return 0;
}
