cmake_minimum_required(VERSION 3.15)
project(libcco VERSION 0.1.0 LANGUAGES C)

set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)
set(CMAKE_C_EXTENSIONS OFF)

add_library(cco
    src/arena.c
    src/diag.c
    src/source.c
    src/lexer.c
    src/value.c
    src/serialize.c
    src/parser.c
    src/type.c
    src/stmt.c
    src/symtab.c
    src/expr.c
    src/template.c
)

target_include_directories(cco PUBLIC include)

if(CMAKE_C_COMPILER_ID MATCHES "GNU|Clang")
    target_compile_options(cco PRIVATE -Wall -Wextra -pedantic)
elseif(MSVC)
    target_compile_options(cco PRIVATE /utf-8)
endif()

option(CCO_BUILD_EXAMPLES "Build example programs" OFF)

if(CCO_BUILD_EXAMPLES)
    macro(add_cco_example name source)
        add_executable(${name} ${source})
        target_link_libraries(${name} cco)
        if(MSVC)
            target_compile_options(${name} PRIVATE /utf-8)
        endif()
    endmacro()

    add_cco_example(example_basic       examples/basic.c)
    add_cco_example(example_parse_simple examples/parse_simple.c)
    add_cco_example(example_template     examples/template_instantiation.c)
    add_cco_example(example_full         examples/full_language.c)
    add_cco_example(example_file_demo    examples/cco_file_demo.c)
    add_cco_example(tree_print          examples/tree_print.c)
endif()

if(BUILD_TESTING)
    enable_testing()

    macro(add_cco_test name source)
        add_executable(${name} ${source})
        target_link_libraries(${name} cco)
        if(MSVC)
            target_compile_options(${name} PRIVATE /utf-8)
        endif()
        add_test(NAME ${name} COMMAND ${name})
    endmacro()

    add_cco_test(test_value    tests/test_value.c)
    add_cco_test(test_serialize tests/test_serialize.c)
    add_cco_test(test_parser   tests/test_parser.c)
    add_cco_test(test_type     tests/test_type.c)
    add_cco_test(test_symtab   tests/test_symtab.c)
    add_cco_test(test_template tests/test_template.c)
    add_cco_test(test_expr     tests/test_expr.c)
    add_cco_test(test_integration tests/test_integration.c)
endif()
