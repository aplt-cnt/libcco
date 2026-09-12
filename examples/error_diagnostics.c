#include <cnt/cco.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int main(void) {
    const char* invalid_text = 
        "server: (\n"
        "    host: \"127.0.0.1\",\n"
        "    port  8080\n" /* Missing colon */
        ")\n";

    printf("Attempting to parse invalid configuration...\n");
    cco_object_t* config = cco_parse_string(invalid_text, strlen(invalid_text), NULL);
    
    if (!config) {
        printf("Parse failed successfully! The document was rejected.\n");
    } else {
        printf("Wait, it succeeded? This shouldn't happen!\n");
        cco_object_release(config);
        return 1;
    }

    return 0;
}
