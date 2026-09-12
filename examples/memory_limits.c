#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <cnt/cco.h>

int main(void)
{
    const char* malicious_payload =
        "data: "
        "(((((((((((((((((((((((((((((((((((((((((((((((())))))))))))))))))))))"
        "))))))))))))))))))))))))))";

    cco_parse_options_t opts;
    opts.max_depth = 5;
    opts.max_string_length = 1024;
    opts.max_document_size = 1048576;
    opts.max_total_allocation = 1024 * 1024;
    opts.max_instantiations = 10;
    opts.max_steps = 1000;

    printf("Parsing deeply nested payload with strict depth limit (5)...\n");
    cco_object_t* obj =
        cco_parse_string(malicious_payload, strlen(malicious_payload), &opts);

    if (!obj)
    {
        cco_error_t err = cco_get_last_error();
        printf("Parse blocked successfully! Error Code: %d\n", err);
    }
    else
    {
        printf("Wait, it succeeded? This shouldn't happen!\n");
        cco_object_release(obj);
        return 1;
    }

    return 0;
}
