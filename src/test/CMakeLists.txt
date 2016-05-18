# we need some of files of our library
set (EALOGGER_SOURCE)
set (EALOGGER_HEADER)

set (TEST_HEADER
    ${CMAKE_CURRENT_SOURCE_DIR}/catch.hpp
)

set (TEST_SOURCE
    ${CMAKE_CURRENT_SOURCE_DIR}/ealogger_test.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/test_utility.cpp
    )

# create a catch executable
add_executable(
    ealogger_test
    ${TEST_HEADER}
    ${TEST_SOURCE}
)

