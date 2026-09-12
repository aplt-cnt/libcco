cmake_minimum_required(VERSION 3.20)
project(libcco VERSION 0.1.0 LANGUAGES C)

set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)
set(CMAKE_C_EXTENSIONS OFF)

option(CCO_ENABLE_FORMAT            "Enable $format dynamic parsing"     OFF)
option(CCO_ENABLE_EVAL              "Enable expression evaluator"        OFF)
option(CCO_ENABLE_COMMENT_PRESERVE  "Preserve comments through parse"    OFF)
option(CCO_ENABLE_ERROR_RECOVERY    "Collect multiple diagnostics"       OFF)
option(CCO_ENABLE_ENV               "Enable $env builtin"                OFF)
option(CCO_ENABLE_STATIC_CALLS      "Enable #T:method / T.method"        ON)
option(CCO_ENABLE_CONSTRUCTORS      "Enable $function.@ constructors"    ON)

add_library(libcco STATIC
    src/hello.c
)

target_include_directories(libcco
    PUBLIC  $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
    PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/src
)

target_compile_options(libcco PRIVATE
    -Wall -Wextra -Werror -Wpedantic -Wconversion -Wshadow
)

foreach(feature
        CCO_ENABLE_FORMAT
        CCO_ENABLE_EVAL
        CCO_ENABLE_COMMENT_PRESERVE
        CCO_ENABLE_ERROR_RECOVERY
        CCO_ENABLE_ENV
        CCO_ENABLE_STATIC_CALLS
        CCO_ENABLE_CONSTRUCTORS)
    if(${feature})
        target_compile_definitions(libcco PUBLIC ${feature}=1)
    else()
        target_compile_definitions(libcco PUBLIC ${feature}=0)
    endif()
endforeach()

enable_testing()
add_subdirectory(tests)
add_subdirectory(benchmarks)
add_subdirectory(examples)