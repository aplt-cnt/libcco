# Benchmark targets.
#
# Benchmarks register only when their source file exists, so a fresh
# clone configures cleanly. They are excluded from CI gates; CI runs
# a short smoke variant instead.

if(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/bench_parse.c")
    add_executable(bench_parse bench_parse.c)
    target_link_libraries(bench_parse PRIVATE libcco)
    target_compile_options(bench_parse PRIVATE -O2)
endif()

if(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/bench_serialize.c")
    add_executable(bench_serialize bench_serialize.c)
    target_link_libraries(bench_serialize PRIVATE libcco)
    target_compile_options(bench_serialize PRIVATE -O2)
endif()
