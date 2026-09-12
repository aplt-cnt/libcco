#include <cnt/cco.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/*
 * Demonstrates the "Lenient Brackets" auto-correction feature.
 * CCO strictly uses () for structural grouping, but users familiar with JSON
 * might mistakenly type {} or []. Lenient mode gracefully auto-corrects them.
 */

int main(void) {
    /* Notice the use of JSON-style brackets which are technically invalid CCO */
    const char* mixed_text = 
        "user: {\n"
        "    name: \"Alice\",\n"
        "    groups: [ \"admin\", \"users\" ]\n"
        "}\n";

    cco_parse_options_t opts;
    opts.max_depth = 256;
    opts.max_string_length = 1048576;
    opts.max_document_size = 10485760;
    opts.max_total_allocation = 10485760;
    opts.max_instantiations = 1000;
    opts.max_steps = 10000;
    opts.allow_colon_instantiation = true;
    opts.lenient_brackets = true; /* Enable auto-correction */

    printf("Parsing invalid CCO text with Lenient Brackets ENABLED...\n");
    cco_object_t* config = cco_parse_string(mixed_text, strlen(mixed_text), &opts);
    
    if (config) {
        printf("Parse succeeded through auto-correction!\n");
        printf("Parsed object is of type: %d\n", cco_object_get_type(config));
        
        /* The parser records warnings via diagnostics */
        cco_error_t err = cco_get_last_error();
        if (err == CCO_OK) {
            printf("A warning was recorded by the parser to indicate auto-correction.\n");
        }
        
        cco_object_release(config);
    } else {
        printf("Parse failed.\n");
        return 1;
    }

    printf("\nNow parsing with Lenient Brackets DISABLED...\n");
    opts.lenient_brackets = false;
    cco_object_t* strict_config = cco_parse_string(mixed_text, strlen(mixed_text), &opts);
    
    if (!strict_config) {
        printf("Parse failed successfully! Error code: %d (CCO_ERR_PARSE)\n", cco_get_last_error());
    } else {
        printf("Wait, this shouldn't succeed.\n");
        cco_object_release(strict_config);
        return 1;
    }

    return 0;
}
