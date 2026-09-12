cmake_minimum_required(VERSION 3.20)
project(libcco C)

set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)
set(CMAKE_C_EXTENSIONS OFF)

# Compiler flags
add_compile_options(
    -Wall
    -Wextra
    -Werror
    -Wpedantic
    -Wconversion
    -Wshadow
)

# Feature flags
option(CCO_ENABLE_FORMAT "Enable $format builtin" OFF)
option(CCO_ENABLE_EVAL "Enable expression evaluation" OFF)
option(CCO_ENABLE_COMMENT_PRESERVE "Preserve comments" OFF)
option(CCO_ENABLE_ERROR_RECOVERY "Enable error recovery" OFF)
option(CCO_ENABLE_ENV "Enable $env builtin" OFF)
option(CCO_ENABLE_STATIC_CALLS "Enable static calls" ON)
option(CCO_ENABLE_CONSTRUCTORS "Enable constructors" ON)

# Source files
set(SOURCES
    src/internal/arena.c
    src/internal/strbuf.c
    src/internal/hash.c
    src/internal/diag.c
    src/internal/lexer.c
    src/internal/object.c
    src/internal/parser.c
    src/internal/symtab.c
    src/internal/inst.c
    src/internal/serializer.c
    src/internal/eval.c
    src/internal/call.c
    src/internal/sandbox.c
    src/internal/cco.c
)

add_library(cco STATIC ${SOURCES})
target_include_directories(cco PUBLIC include PRIVATE src)

# Add custom target for linting
add_custom_target(lint
    COMMAND clang-format --dry-run -Werror include/cnt/*.h src/internal/*.c src/internal/*.h test/*.c 2>/dev/null || true
    COMMENT "Running clang-format"
)