#include "../test.h"
#include "arena.h"

void test_arena(void) {
    cco_arena_t arena;
    cco_arena_init(&arena, 1024);
    
    void* p1 = cco_arena_alloc(&arena, 100);
    EXPECT_TRUE(p1 != NULL);
    
    void* p2 = cco_arena_alloc(&arena, 200);
    EXPECT_TRUE(p2 != NULL);
    EXPECT_TRUE(p1 != p2);
    
    /* Should fail to allocate exceeding max */
    void* p3 = cco_arena_alloc(&arena, 2048);
    EXPECT_TRUE(p3 == NULL);
    
    cco_arena_destroy(&arena);
}
