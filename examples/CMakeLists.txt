set(EALOGGER_BASIC_SOURCE
    ${CMAKE_CURRENT_SOURCE_DIR}/ealogger_basic.cpp
)
set(EALOGGER_BENCH_SOURCE
    ${CMAKE_CURRENT_SOURCE_DIR}/ealogger_bench.cpp
)

set()

include_directories(
    ${CMAKE_CURRENT_BINARY_DIR}
    "../include"
)

# configure a header file to pass some of the CMake settings
# to the source code
configure_file (
    ${CMAKE_SOURCE_DIR}/include/ealogger/config.h.in
    ${CMAKE_CURRENT_BINARY_DIR}/config.h
)

if(BUILD_EXAMPLES)
    message(STATUS "Will build the example applications for ealogger")
    add_executable(ealogger_basic ${EALOGGER_BASIC_SOURCE})
    add_executable(ealogger_bench ${EALOGGER_BENCH_SOURCE})
    target_link_libraries(ealogger_basic ealogger)
    target_link_libraries(ealogger_bench ealogger)
endif(BUILD_EXAMPLES)
