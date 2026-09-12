#include "../test.h"
#include "strbuf.h"
#include "arena.h"

void test_strbuf(void) {
    cco_arena_t arena;
    cco_arena_init(&arena, 1024 * 1024);
    
    cco_strbuf_t sb;
    cco_strbuf_init(&sb, &arena);
    
    cco_error_t err = cco_strbuf_append(&sb, "Hello", 5);
    EXPECT_EQ_INT(0, err);
    EXPECT_EQ_INT(5, sb.length);
    EXPECT_TRUE(strncmp(sb.data, "Hello", 5) == 0);
    
    err = cco_strbuf_append(&sb, "!", 1);
    EXPECT_EQ_INT(0, err);
    EXPECT_EQ_INT(6, sb.length);
    EXPECT_TRUE(strncmp(sb.data, "Hello!", 6) == 0);
    
    cco_arena_destroy(&arena);
}
