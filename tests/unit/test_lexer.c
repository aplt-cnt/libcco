#include "../test.h"
#include "lexer.h"
#include "arena.h"
#include <cnt/cco_options.h>

void test_lexer(void) {
    cco_arena_t arena;
    cco_arena_init(&arena, 1024 * 1024);
    
    cco_parse_options_t opts;
    cco_parse_options_init(&opts);
    
    cco_lexer_t lexer;
    const char* src = "true false 123 none";
    cco_lexer_init(&lexer, src, strlen(src), &opts, &arena);
    
    cco_token_t tok;
    
    cco_lexer_next(&lexer, &tok);
    EXPECT_EQ_INT(CCO_TOK_TRUE, tok.type);
    
    cco_lexer_next(&lexer, &tok);
    EXPECT_EQ_INT(CCO_TOK_FALSE, tok.type);
    
    cco_lexer_next(&lexer, &tok);
    EXPECT_EQ_INT(CCO_TOK_INTEGER, tok.type);
    
    cco_lexer_next(&lexer, &tok);
    EXPECT_EQ_INT(CCO_TOK_IDENTIFIER, tok.type);
    
    cco_lexer_next(&lexer, &tok);
    EXPECT_EQ_INT(CCO_TOK_EOF, tok.type);
    
    cco_arena_destroy(&arena);
}
