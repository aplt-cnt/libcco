#include "test.h"
#include <stdio.h>

int tests_run = 0;
int tests_passed = 0;

/* Declare test suites */
extern void test_arena(void);
extern void test_strbuf(void);
extern void test_lexer(void);
extern void test_parser(void);

int main(void) {
    printf("Starting libcco unit tests...\n");

    test_arena();
    test_strbuf();
    test_lexer();
    test_parser();

    printf("========================================\n");
    printf("Tests run:    %d\n", tests_run);
    printf("Tests passed: %d\n", tests_passed);
    printf("Tests failed: %d\n", tests_run - tests_passed);

    if (tests_run == tests_passed) {
        printf("SUCCESS!\n");
        return 0;
    } else {
        printf("FAILURE!\n");
        return 1;
    }
}
