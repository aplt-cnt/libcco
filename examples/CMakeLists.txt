# Example targets.
#
# Examples register only when their source file exists, so a fresh
# clone configures cleanly.

set(CCO_C_EXAMPLES
    basic
    parse_simple
    full_language
    template_instantiation
    cco_file_demo
    tree_print
)

set(CCO_CPP_EXAMPLES
    basic
    parse_simple
    full_language
    comparison
    error_handling
    array_ops
)

foreach(e IN LISTS CCO_C_EXAMPLES)
    if(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/${e}.c")
        add_executable(example_${e} ${e}.c)
        target_link_libraries(example_${e} PRIVATE libcco)
    endif()
endforeach()

foreach(e IN LISTS CCO_CPP_EXAMPLES)
    if(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/${e}.cpp")
        add_executable(example_${e} ${e}.cpp)
        target_link_libraries(example_${e} PRIVATE libcco)
    endif()
endforeach()
