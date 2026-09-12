# Unit tests.
#
# Unity is vendored under tests/third_party/unity once implementation
# lands. Until then this file only registers tests that actually exist,
# so a fresh clone configures cleanly.

set(CCO_UNIT_TESTS)

if(CCO_UNIT_TESTS)
    if(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/third_party/unity/src/unity.c")
        add_library(unity STATIC
            third_party/unity/src/unity.c)
        target_include_directories(unity PUBLIC
            ${CMAKE_CURRENT_SOURCE_DIR}/third_party/unity/src)
    endif()

    foreach(t IN LISTS CCO_UNIT_TESTS)
        if(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/unit/${t}.c")
            add_executable(${t} unit/${t}.c)
            target_link_libraries(${t} PRIVATE libcco unity)
            add_test(NAME ${t} COMMAND ${t})
        endif()
    endforeach()
endif()
