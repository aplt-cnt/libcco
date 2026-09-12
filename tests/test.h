#ifndef TEST_H
#define TEST_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

extern int tests_run;
extern int tests_passed;

#define EXPECT_TRUE(cond) \
    do { \
        tests_run++; \
        if (cond) { tests_passed++; } \
        else { fprintf(stderr, "FAIL: %s:%d: EXPECT_TRUE(%s)\n", __FILE__, __LINE__, #cond); } \
    } while(0)

#define EXPECT_EQ_INT(expected, actual) \
    do { \
        tests_run++; \
        int e = (expected); int a = (actual); \
        if (e == a) { tests_passed++; } \
        else { fprintf(stderr, "FAIL: %s:%d: Expected %d, got %d\n", __FILE__, __LINE__, e, a); } \
    } while(0)

#define EXPECT_EQ_STR(expected, actual) \
    do { \
        tests_run++; \
        const char* e = (expected); const char* a = (actual); \
        if (e && a && strcmp(e, a) == 0) { tests_passed++; } \
        else if (!e && !a) { tests_passed++; } \
        else { fprintf(stderr, "FAIL: %s:%d: Expected '%s', got '%s'\n", __FILE__, __LINE__, e?e:"NULL", a?a:"NULL"); } \
    } while(0)

#endif /* TEST_H */
