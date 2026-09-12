#include <cnt/cco.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/*
 * Demonstrates CCO's security features for rejecting malicious inputs
 * by configuring memory and depth limits.
 */

int main(void) {
    const char* malicious_payload = 
        "data: [[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[[]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]]";

    cco_parse_options_t opts;
    /* Initialize with defaults */
    opts.max_depth = 5; /* Very low depth limit */
    opts.max_string_length = 1024;
    opts.max_document_size = 1048576;
    opts.max_total_allocation = 1024 * 1024; /* 1 MB */
    opts.max_instantiations = 10;
    opts.max_steps = 1000;
    
    printf("Parsing deeply nested payload with strict depth limit (5)...\n");
    cco_object_t* obj = cco_parse_string(malicious_payload, strlen(malicious_payload), &opts);
    
    if (!obj) {
        cco_error_t err = cco_get_last_error();
        printf("Parse blocked successfully! Error Code: %d\n", err);
        if (err == CCO_ERR_OUT_OF_RANGE) {
            printf("(CCO_ERR_OUT_OF_RANGE: Depth or Memory limit exceeded)\n");
        }
    } else {
        printf("Wait, it succeeded? This shouldn't happen!\n");
        cco_object_release(obj);
        return 1;
    }

    return 0;
}
