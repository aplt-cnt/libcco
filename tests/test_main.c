#include <cnt/cco.h>
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>

void test_arena_initialization(void) {
    printf("Running test_arena_initialization...\n");
    cco_object_t* obj = cco_parse_string("true", 4, NULL);
    if (!obj) {
        printf("Parse error: %d\n", cco_get_last_error());
    }
    assert(obj != NULL);
    cco_object_release(obj);
    printf("Passed test_arena_initialization.\n");
}

int main(void) {
    printf("Starting libcco unit tests...\n");
    test_arena_initialization();
    printf("All tests passed successfully.\n");
    return EXIT_SUCCESS;
}
