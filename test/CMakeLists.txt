set(EALOGGER_TESTSOURCE
    ${CMAKE_CURRENT_SOURCE_DIR}/ealogger_tester.cpp
)

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

if(BUILD_TEST)
    message(STATUS "Will build the tester application for ealogger")
    add_executable(ealogger_test ${EALOGGER_TESTSOURCE})
    target_link_libraries(ealogger_test ealogger)
endif(BUILD_TEST)
